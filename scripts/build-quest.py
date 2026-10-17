#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Build the Quest APK and its OpenXR loader from pinned sources (Python 3.9+)."""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys
from builder_support import prepare_environment, validate_environment

ROOT = Path(__file__).resolve().parents[1]
NDK_VERSION = "27.0.12077973"
CMAKE_VERSION = "3.22.1"

def run(args, cwd=ROOT):
    subprocess.run([str(a) for a in args], cwd=cwd, check=True)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sdk", help="Android SDK directory")
    parser.add_argument("--java", help="JDK 17 directory (JAVA_HOME)")
    parser.add_argument("--check", action="store_true", help="Check prerequisites without building")
    args = parser.parse_args()
    environment = prepare_environment(args.sdk, args.java)
    os.environ.update(environment)
    errors, messages = validate_environment(ROOT, environment)
    for message in messages:
        print(message, flush=True)
    if errors:
        raise SystemExit("\n".join(errors))
    if args.check:
        return
    if args.sdk:
        # Explicit GUI selection takes precedence over a previous local SDK path.
        properties = ROOT / "android/local.properties"
        lines = properties.read_text().splitlines() if properties.exists() else []
        lines = [line for line in lines if not line.strip().startswith("sdk.dir=")]
        sdk_path = Path(environment["ANDROID_HOME"]).as_posix().replace(" ", "\\ ")
        lines.append("sdk.dir=" + sdk_path)
        properties.write_text("\n".join(lines) + "\n")
    sdk_value = os.environ.get("ANDROID_HOME") or os.environ.get("ANDROID_SDK_ROOT")
    if not sdk_value:
        raise SystemExit("Set ANDROID_HOME to your Android SDK directory. See docs/BUILD.md.")
    sdk = Path(sdk_value).expanduser().resolve()
    suffix = ".exe" if os.name == "nt" else ""
    cmake = sdk / "cmake" / CMAKE_VERSION / "bin" / ("cmake" + suffix)
    ninja = cmake.with_name("ninja" + suffix)
    toolchain = sdk / "ndk" / NDK_VERSION / "build/cmake/android.toolchain.cmake"
    for required in (cmake, ninja, toolchain):
        if not required.is_file():
            raise SystemExit(f"Missing SDK component: {required}. See docs/BUILD.md.")
    run(["git", "submodule", "update", "--init", "--recursive", "--jobs", "4"])
    build = ROOT / ".build/openxr-arm64"
    run([cmake, "-S", ROOT / "third_party/openxr-sdk-source", "-B", build,
         "-G", "Ninja", f"-DCMAKE_MAKE_PROGRAM={ninja}",
         f"-DCMAKE_TOOLCHAIN_FILE={toolchain}", "-DANDROID_ABI=arm64-v8a",
         "-DANDROID_PLATFORM=android-28", "-DCMAKE_BUILD_TYPE=Release",
         "-DBUILD_LOADER=ON", "-DDYNAMIC_LOADER=ON", "-DBUILD_API_LAYERS=OFF",
         "-DBUILD_TESTS=OFF", "-DBUILD_CONFORMANCE_TESTS=OFF"])
    run([cmake, "--build", build, "--target", "openxr_loader", "--parallel", "4"])
    candidates = list(build.rglob("libopenxr_loader.so"))
    if len(candidates) != 1:
        raise SystemExit(f"Expected one loader, found {len(candidates)}")
    target = ROOT / "android/app/src/main/jniLibs/arm64-v8a/libopenxr_loader.so"
    target.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(candidates[0], target)
    android = ROOT / "android"
    gradlew = android / ("gradlew.bat" if os.name == "nt" else "gradlew")
    run([gradlew, "assembleDebug", "--console=plain", "--no-daemon"], cwd=android)
    apk = android / "app/build/outputs/apk/debug/xr.rttr-Quest.apk"
    if not apk.is_file():
        raise SystemExit("Build returned without producing the expected APK.")
    print("APK:", apk, flush=True)

if __name__ == "__main__":
    try:
        main()
    except subprocess.CalledProcessError as error:
        sys.exit(error.returncode)
