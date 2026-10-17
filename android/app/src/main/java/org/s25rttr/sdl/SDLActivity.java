package org.s25rttr.sdl;

import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.system.Os;
import android.view.WindowInsets;
import android.view.WindowInsetsController;

public class SDLActivity extends org.libsdl.app.SDLActivity {
    // Quest scene model (room scan) for placing the board on a real table; requested only when the option is enabled.
    private static final String SCENE_PERMISSION = "com.oculus.permission.USE_SCENE";
    private static final int SCENE_PERMISSION_CODE = 4711;
    /** Process-wide guard: the start route is redirected at most once (see onCreate). */
    private static boolean sRedirected = false;

    /** Called from the OpenXR driver (JNI). */
    public static boolean hasScenePermission() {
        return mSingleton != null && mSingleton.checkSelfPermission(SCENE_PERMISSION) == PackageManager.PERMISSION_GRANTED;
    }

    /** Called from the OpenXR driver (JNI); shows the system dialog once, without blocking the game thread. */
    public static void requestScenePermission() {
        final SDLActivity activity = (SDLActivity) mSingleton;
        if(activity == null) return;
        activity.runOnUiThread(() -> activity.requestPermissions(new String[] {SCENE_PERMISSION}, SCENE_PERMISSION_CODE));
    }

    // --- Video recording of the game image (called from the OpenXR and SDL audio drivers via JNI) ---
    public static android.view.Surface recorderStart(int width, int height, int fps) {
        String base = Os.getenv("RTTR_PREFIX_DIR");
        return VideoRecorder.start((base != null ? base : "/sdcard/RTTR") + "/videos", width, height, fps);
    }
    public static void recorderStop() { VideoRecorder.stop(); }
    public static void recorderConfigureAudio(int rate, int channels) { VideoRecorder.configureAudio(rate, channels); }
    public static void recorderAudio(java.nio.ByteBuffer pcm, int bytes) { VideoRecorder.pushAudio(pcm, bytes); }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // After a crash Android may recreate this activity directly in a fresh process. The paths are only set by
        // GameStartActivity, and the engine aborts without them, so take the regular start route instead.
        // Os.getenv reads the live native environment (GameStartActivity sets it via Os.setenv); System.getenv may
        // return a snapshot. The intent extra cannot be used as the only criterion: Android recreates the activity
        // with its original intent. Instead at most one redirect per process is allowed, so a loop is impossible.
        if(Os.getenv("RTTR_PREFIX_DIR") == null && !sRedirected) {
            sRedirected = true;
            super.onCreate(savedInstanceState);
            startActivity(new Intent(this, GameStartActivity.class));
            finish();
            return;
        }
        super.onCreate(savedInstanceState);
        SetFullscreen();
    }

    @Override
    protected void onResume() {
        super.onResume();
        SetFullscreen();
    }

    @Override
    public void setOrientationBis(int w, int h, boolean resizable, String hint) {
        super.setOrientationBis(w, h, resizable, hint);
        mSingleton.setRequestedOrientation(getIntent().getIntExtra("orientation", ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE));
    }

    @Override
    protected String[] getLibraries() {
        return new String[] {
                "SDL2",
                "SDL2_mixer",
                "videoSDL2",
                "audioSDL",
                "s25client"
        };
    }

    private void SetFullscreen() {
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            getWindow().setDecorFitsSystemWindows(false);
            WindowInsetsController controller = getWindow().getInsetsController();
            if(controller != null) {
                controller.hide(WindowInsets.Type.systemBars());
                controller.setSystemBarsBehavior(WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
            }
        }
    }
}
