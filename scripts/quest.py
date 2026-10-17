#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""ADB helpers. Never bundles original game data or deletes savegames."""
import argparse
import os
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]

def find_adb():
    if os.environ.get("ADB"):
        return os.environ["ADB"]
    sdk = os.environ.get("ANDROID_HOME") or os.environ.get("ANDROID_SDK_ROOT")
    if sdk:
        path = Path(sdk) / "platform-tools" / ("adb.exe" if os.name == "nt" else "adb")
        if path.is_file():
            return str(path)
    path = shutil.which("adb")
    if path:
        return path
    raise SystemExit("ADB not found. Set ANDROID_HOME, ADB, or add adb to PATH.")

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--serial", help="ADB serial if multiple devices are connected")
    commands = parser.add_subparsers(dest="command", required=True)
    install = commands.add_parser("install", help="Install APK and launch (save your game first)")
    install.add_argument("apk", nargs="?", type=Path,
                         default=ROOT / "android/app/build/outputs/apk/debug/xr.rttr-Quest.apk")
    data = commands.add_parser("import-data", help="Copy your DATA/GFX; requires explicit overwrite option if present")
    data.add_argument("folder", type=Path)
    data.add_argument("--replace-data", action="store_true", help="Allow overwriting existing original game data")
    args = parser.parse_args()
    adb = [find_adb()]
    if args.serial:
        adb += ["-s", args.serial]
    def run(*parts, capture=False):
        return subprocess.run(adb + list(parts), check=True, text=True, capture_output=capture)
    if args.command == "install":
        if not args.apk.is_file():
            parser.error(f"APK not found: {args.apk}")
        run("get-state")
        run("install", "-r", str(args.apk.resolve()))
        run("shell", "am", "start", "-n", "xr.rttr/org.s25rttr.sdl.GameStartActivity")
    else:
        for name in ("DATA", "GFX"):
            if not (args.folder / name).is_dir():
                parser.error(f"Missing directory: {args.folder / name}")
        run("get-state")
        existing = run("shell", "if [ -d /sdcard/RTTR/S2/DATA ] || [ -d /sdcard/RTTR/S2/GFX ]; then echo PRESENT; fi", capture=True)
        if "PRESENT" in existing.stdout and not args.replace_data:
            parser.error("Game data already exists. Use --replace-data only to deliberately overwrite it.")
        run("shell", "mkdir", "-p", "/sdcard/RTTR/S2")
        for name in ("DATA", "GFX"):
            run("push", str((args.folder / name).resolve()), "/sdcard/RTTR/S2/")
        print("Copied DATA/GFX. Select /sdcard/RTTR/S2 in the app. Content compatibility is not yet validated.")

if __name__ == "__main__":
    main()
