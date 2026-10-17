package org.s25rttr.sdl;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.media.MediaMuxer;
import android.os.Bundle;
import android.util.Log;
import android.view.Surface;

import java.io.File;
import java.nio.ByteBuffer;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

/**
 * Records the flat game image (from the OpenXR driver via an input surface) plus the mixed game sound
 * (from the SDL audio driver) into an MP4 under RTTR/videos. Driven from native code via SDLActivity.
 */
final class VideoRecorder {
    private static final String TAG = "RTTR-XR";
    private static final Object LOCK = new Object();

    private static MediaCodec video, audio;
    private static MediaMuxer muxer;
    private static Surface inputSurface;
    private static Thread drainThread;
    private static int videoTrack = -1, audioTrack = -1;
    private static boolean muxerStarted;
    private static volatile boolean running, stopRequested;
    private static volatile int sampleRate = 44100, channels = 2;
    private static long firstAudioUs = -1, audioFrames;
    private static String path;

    private VideoRecorder() {}

    static void configureAudio(int rate, int channelCount) {
        if(rate > 0) sampleRate = rate;
        if(channelCount > 0) channels = channelCount;
    }

    /** Starts video and audio encoders; returns the surface the driver renders into, or null on failure. */
    static Surface start(String directory, int width, int height, int fps) {
        synchronized(LOCK) {
            if(running) return inputSurface;
            // The previous file is still being finalised: refuse instead of sharing the codecs (the menu shows "failed").
            if(drainThread != null && drainThread.isAlive()) { Log.w(TAG, "Recording still finalising"); return null; }
            try {
                File dir = new File(directory);
                if(!dir.isDirectory() && !dir.mkdirs()) throw new IllegalStateException("cannot create " + directory);
                path = new File(dir, "xr.rttr-" + new SimpleDateFormat("yyyyMMdd-HHmmss", Locale.ROOT).format(new Date()) + ".mp4").getPath();

                MediaFormat vf = MediaFormat.createVideoFormat(MediaFormat.MIMETYPE_VIDEO_AVC, width, height);
                vf.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);
                vf.setInteger(MediaFormat.KEY_BIT_RATE, Math.max(4_000_000, width * height * 6));
                vf.setInteger(MediaFormat.KEY_FRAME_RATE, fps);
                vf.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 2);
                video = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_VIDEO_AVC);
                video.configure(vf, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
                inputSurface = video.createInputSurface();

                MediaFormat af = MediaFormat.createAudioFormat(MediaFormat.MIMETYPE_AUDIO_AAC, sampleRate, channels);
                af.setInteger(MediaFormat.KEY_AAC_PROFILE, MediaCodecInfo.CodecProfileLevel.AACObjectLC);
                af.setInteger(MediaFormat.KEY_BIT_RATE, 192_000);
                af.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, 32 * 1024);
                audio = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_AUDIO_AAC);
                audio.configure(af, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);

                muxer = new MediaMuxer(path, MediaMuxer.OutputFormat.MUXER_OUTPUT_MPEG_4);
                videoTrack = audioTrack = -1;
                muxerStarted = false;
                firstAudioUs = -1;
                audioFrames = 0;
                video.start();
                audio.start();
                stopRequested = false;
                running = true;
                drainThread = new Thread(VideoRecorder::drainLoop, "VideoRecorder");
                drainThread.start();
                Log.i(TAG, "Recording started: " + path + " " + width + "x" + height + "@" + fps);
                return inputSurface;
            } catch(Exception e) {
                Log.e(TAG, "Recording start failed", e);
                releaseAll();
                return null;
            }
        }
    }

    /** Requests the end of the recording; the file is finalised asynchronously. */
    static void stop() {
        synchronized(LOCK) {
            if(!running) return;
            running = false;
            stopRequested = true;
        }
    }

    static String lastPath() { return path; }

    /** Mixed game sound, 16-bit PCM interleaved; called on the SDL audio thread. Dropped when not recording. */
    static void pushAudio(ByteBuffer pcm, int bytes) {
        final MediaCodec audio = VideoRecorder.audio; // may be released concurrently by the drain thread
        if(!running || audio == null) return;
        try {
            final int frameBytes = 2 * channels;
            if(firstAudioUs < 0) firstAudioUs = System.nanoTime() / 1000;
            int offset = 0;
            while(offset < bytes) {
                final int index = audio.dequeueInputBuffer(0);
                if(index < 0) return; // encoder busy: drop the rest of this chunk
                ByteBuffer in = audio.getInputBuffer(index);
                if(in == null) return;
                in.clear();
                final int chunk = Math.min(in.remaining() / frameBytes * frameBytes, bytes - offset);
                ByteBuffer slice = pcm.duplicate();
                slice.position(offset);
                slice.limit(offset + chunk);
                in.put(slice);
                final long pts = firstAudioUs + audioFrames * 1_000_000L / sampleRate;
                audio.queueInputBuffer(index, 0, chunk, pts, 0);
                audioFrames += chunk / frameBytes;
                offset += chunk;
            }
        } catch(Exception e) {
            Log.w(TAG, "Audio frame dropped", e);
        }
    }

    private static void drainLoop() {
        final MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();
        boolean videoDone = false, audioDone = false, eosSent = false;
        try {
            while(!(videoDone && audioDone)) {
                if(stopRequested && !eosSent) {
                    eosSent = true;
                    video.signalEndOfInputStream();
                    final int index = audio.dequeueInputBuffer(10_000);
                    if(index >= 0) audio.queueInputBuffer(index, 0, 0, System.nanoTime() / 1000, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                    else audioDone = true;
                }
                if(!videoDone) videoDone = drain(video, info, true);
                if(!audioDone) audioDone = drain(audio, info, false);
            }
        } catch(Exception e) {
            Log.e(TAG, "Recording failed", e);
        } finally {
            synchronized(LOCK) {
                releaseAll();
            }
            Log.i(TAG, "Recording finished: " + path);
        }
    }

    /** Returns true once the codec delivered end of stream. */
    private static boolean drain(MediaCodec codec, MediaCodec.BufferInfo info, boolean isVideo) {
        final int index = codec.dequeueOutputBuffer(info, 5_000);
        if(index == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
            synchronized(LOCK) {
                if(isVideo) videoTrack = muxer.addTrack(codec.getOutputFormat());
                else audioTrack = muxer.addTrack(codec.getOutputFormat());
                if(videoTrack >= 0 && audioTrack >= 0 && !muxerStarted) {
                    muxer.start();
                    muxerStarted = true;
                    // Frames before the muxer start are dropped; ask for a key frame so the video starts cleanly.
                    Bundle sync = new Bundle();
                    sync.putInt(MediaCodec.PARAMETER_KEY_REQUEST_SYNC_FRAME, 0);
                    video.setParameters(sync);
                }
            }
            return false;
        }
        if(index < 0) return false;
        final boolean eos = (info.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0;
        if((info.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) == 0 && info.size > 0 && muxerStarted) {
            ByteBuffer out = codec.getOutputBuffer(index);
            if(out != null) {
                out.position(info.offset);
                out.limit(info.offset + info.size);
                muxer.writeSampleData(isVideo ? videoTrack : audioTrack, out, info);
            }
        }
        codec.releaseOutputBuffer(index, false);
        return eos;
    }

    private static void releaseAll() {
        running = false;
        try { if(video != null) { video.stop(); } } catch(Exception ignored) {}
        try { if(audio != null) { audio.stop(); } } catch(Exception ignored) {}
        try { if(muxer != null && muxerStarted) muxer.stop(); } catch(Exception ignored) {}
        if(video != null) video.release();
        if(audio != null) audio.release();
        if(muxer != null) muxer.release();
        if(inputSurface != null) inputSurface.release();
        video = audio = null;
        muxer = null;
        inputSurface = null;
        muxerStarted = false;
    }
}
