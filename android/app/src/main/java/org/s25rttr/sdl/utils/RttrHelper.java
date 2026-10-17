package org.s25rttr.sdl.utils;

import android.content.Context;
import android.system.ErrnoException;
import android.system.Os;

import org.s25rttr.sdl.data.Path;
import org.s25rttr.sdl.data.Settings;

import java.io.File;

public class RttrHelper {
    // S2 game files rttr is actually using
    private static final String[] S2Assets = {
            "DATA",
            "GFX/PICS",
            "GFX/PICS/MISSION",
            "DATA/MAPS",
            "DATA/MAPS2",
            "DATA/MAPS3",
            "DATA/MAPS4",
            "DATA/MBOB",
            "GFX/TEXTURES",
            "DATA/SOUNDDAT/SOUND.LST",
            "DATA/BOBS/BOAT.LST",
            "DATA/BOOT_Z.LST",
            "DATA/BOBS/CARRIER.BOB",
            "DATA/IO/IO.DAT",
            "DATA/BOBS/JOBS.BOB",
            "DATA/MIS0BOBS.LST",
            "DATA/MIS1BOBS.LST",
            "DATA/MIS2BOBS.LST",
            "DATA/MIS3BOBS.LST",
            "DATA/MIS4BOBS.LST",
            "DATA/MIS5BOBS.LST",
            "GFX/PALETTE/PAL5.BBM",
            "GFX/PALETTE/PAL6.BBM",
            "GFX/PALETTE/PAL7.BBM",
            "GFX/PALETTE/PALETTI0.BBM",
            "GFX/PALETTE/PALETTI1.BBM",
            "GFX/PALETTE/PALETTI8.BBM",
            "DATA/RESOURCE.DAT",
            "DATA/CBOB/ROM_BOBS.LST",
            "GFX/PICS/SETUP013.LBM",
            "GFX/PICS/SETUP015.LBM"
    };


    public static Path GetDriverDir(Context context) {
        return new Path(context.getCacheDir().toString() ).Append("/driver");
    }

    public static Path GetLibDir(Context context) {
        return new Path(context.getApplicationInfo().nativeLibraryDir);
    }

    public static Path GetLogDir(Settings settings) {
        return new Path(settings.RttrDirectory).Append(".s25rttr/LOGS");
    }

    public static boolean PrepareDrivers(Context context) {
        File libDir = new File(GetLibDir(context).toString());
        File drDir = new File(GetDriverDir(context).toString());

        File vidDir = new File(drDir, "video");
        File audDir = new File(drDir, "audio");

        boolean vidSuccess = vidDir.exists() || vidDir.mkdirs();
        boolean audSuccess = audDir.exists() || audDir.mkdirs();

        if(!vidSuccess || !audSuccess)
            return false;

        // Alle im Paket enthaltenen Treiber verlinken (u. a. videoOpenXR).
        // Always deleting prevents weird errors after app update
        File[] driverLibs = libDir.listFiles();
        if(driverLibs == null)
            return false;
        try {
            for(File lib : driverLibs) {
                String name = lib.getName();
                File destDir = null;
                if(name.startsWith("libvideo") && name.endsWith(".so"))
                    destDir = vidDir;
                else if(name.startsWith("libaudio") && name.endsWith(".so"))
                    destDir = audDir;
                if(destDir == null)
                    continue;
                File dest = new File(destDir, name);
                dest.delete();
                Os.symlink(lib.getAbsolutePath(), dest.getAbsolutePath());
            }
        } catch (ErrnoException ignore) {
            return false;
        }

        // Mindestens die bisherigen Standardtreiber muessen greifen
        File vidDest = new File(vidDir, "libvideoSDL2.so");
        File audDest = new File(audDir, "libaudioSDL.so");
        return vidDest.exists() && audDest.exists();
    }

    public static boolean CheckS2Files(Settings settings) {
        return CheckS2Files(settings.GameDirectory);
    }
    public static boolean CheckS2Files(String path) {
        if(!path.endsWith("/"))
            path += "/";

        for(String asset : S2Assets) {
            if(!new File(path + asset).exists())
                return false;
        }
        return true;
    }

    /**
     * Tries to find s2 installation at old location
     * @param settings
     * @return <code>S2 path</code> if found,
     * <code>null</code> otherwise
     */
    public static String COMPAT_FindS2Installation(Settings settings) {
        Path path = new Path(settings.RttrDirectory).Append("share/s25rttr/S2");
        if(path.Exists() && CheckS2Files(path.toString()))
            return path.toString();
        return null;
    }
}
