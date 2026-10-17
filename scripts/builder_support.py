# SPDX-License-Identifier: GPL-2.0-or-later
"""Shared preflight for the CLI and graphical Quest builder."""
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys

NDK_VERSION = "27.0.12077973"
CMAKE_VERSION = "3.22.1"
# s25client/CMakeLists.txt: find_package(Gettext REQUIRED) needs both tools to compile the translations.
GETTEXT_TOOLS = ("msgfmt", "msgmerge")
GETTEXT_HINTS = {
    "win32": "Windows: winget install --id mlocati.GetText (danach Builder neu öffnen)",
    "darwin": "macOS: brew install gettext",
    "linux": "Linux: Paket gettext installieren, z. B. sudo apt install gettext",
}


def prepare_environment(sdk=None, java=None):
    env = os.environ.copy()
    if sdk:
        env["ANDROID_HOME"] = str(Path(sdk).expanduser().resolve())
    elif not env.get("ANDROID_HOME") and env.get("ANDROID_SDK_ROOT"):
        env["ANDROID_HOME"] = env["ANDROID_SDK_ROOT"]
    if env.get("ANDROID_HOME"):
        env["ANDROID_SDK_ROOT"] = env["ANDROID_HOME"]
    if java:
        home = Path(java).expanduser().resolve()
        if (home / "Contents/Home").is_dir():
            home /= "Contents/Home"
        env["JAVA_HOME"] = str(home)
    if env.get("JAVA_HOME"):
        env["PATH"] = str(Path(env["JAVA_HOME"]) / "bin") + os.pathsep + env.get("PATH", "")
    env["PYTHONUNBUFFERED"] = "1"
    return env


def validate_environment(root, env):
    errors, messages = [], []
    git = shutil.which("git", path=env.get("PATH"))
    if not git:
        errors.append("Git fehlt. Git installieren und den Builder danach neu öffnen.")
    elif not (root / ".git").exists():
        errors.append("Kein Git-Checkout. Repository mit git clone beziehen; ein ZIP reicht nicht.")
    else:
        try:
            result = subprocess.run([git, "--version"], env=env, capture_output=True, text=True, timeout=15)
            if result.returncode:
                errors.append("Git lässt sich nicht starten.")
            else:
                messages.append("OK: " + result.stdout.strip())
        except (OSError, subprocess.TimeoutExpired) as error:
            errors.append("Git konnte nicht geprüft werden: " + str(error))
    java_home = env.get("JAVA_HOME")
    if not java_home:
        errors.append("JDK-17-Ordner auswählen oder JAVA_HOME setzen.")
    else:
        java = Path(java_home) / "bin" / ("java.exe" if os.name == "nt" else "java")
        javac = java.with_name("javac.exe" if os.name == "nt" else "javac")
        if not java.is_file() or not javac.is_file():
            errors.append("Kein vollständiges JDK: java und javac müssen im bin-Ordner vorhanden sein.")
        else:
            try:
                result = subprocess.run([str(java), "-version"], env=env, capture_output=True, text=True, timeout=15)
                version = re.search(r'version\s+"(\d+)', result.stderr + result.stdout)
                if result.returncode or not version or version.group(1) != "17":
                    errors.append("Bitte JDK 17 auswählen. Die ausgewählte Java-Version passt nicht zur geprüften Umgebung.")
                else:
                    messages.append("OK: JDK 17 – " + java_home)
            except (OSError, subprocess.TimeoutExpired) as error:
                errors.append("Java konnte nicht geprüft werden: " + str(error))
    sdk = env.get("ANDROID_HOME")
    if not sdk:
        errors.append("Android-SDK-Ordner auswählen oder ANDROID_HOME setzen.")
    else:
        base = Path(sdk)
        suffix = ".exe" if os.name == "nt" else ""
        required = {
            "Android SDK Platform 36": base / "platforms/android-36/android.jar",
            "Android Build-Tools 35.0.0": base / "build-tools/35.0.0" / ("aapt2" + suffix),
            f"NDK {NDK_VERSION}": base / "ndk" / NDK_VERSION / "build/cmake/android.toolchain.cmake",
            f"CMake {CMAKE_VERSION}": base / "cmake" / CMAKE_VERSION / "bin" / ("cmake" + suffix),
            "Ninja": base / "cmake" / CMAKE_VERSION / "bin" / ("ninja" + suffix),
        }
        for label, path in required.items():
            if path.is_file():
                messages.append("OK: " + label)
            else:
                errors.append("Fehlt: " + label + " – im Android SDK Manager installieren.")
        if not (base / "licenses/android-sdk-license").is_file():
            errors.append("Android-SDK-Lizenz fehlt. Im SDK Manager bzw. mit sdkmanager --licenses prüfen und akzeptieren.")
    errors_gettext, messages_gettext = check_gettext(env)
    return errors + errors_gettext, messages + messages_gettext


def check_gettext(env, platform=None):
    """The engine build fails without msgfmt/msgmerge, so a missing tool is an error, not a warning."""
    missing = [tool for tool in GETTEXT_TOOLS if not shutil.which(tool, path=env.get("PATH"))]
    if not missing:
        return [], ["OK: gettext (" + ", ".join(GETTEXT_TOOLS) + ")"]
    platform = platform or sys.platform
    hint = next((text for key, text in GETTEXT_HINTS.items() if platform.startswith(key)),
                "; ".join(GETTEXT_HINTS.values()))
    return ["Fehlt: gettext (" + ", ".join(missing) + ") im PATH – nötig für die Übersetzungen der Engine. "
            + hint + "."], []


def gettext_status(line):
    """Turn one --check output line into the builder's gettext status row: (ok, text), or None if unrelated.

    The row repeats only tool names and the install hint, never paths."""
    line = line.strip()
    if line.startswith("OK: gettext"):
        return True, "gettext: OK"
    # Tolerant separator: a child writing in the console code page turns "–" into U+FFFD or "?".
    match = re.match(r"Fehlt: gettext \(([^)]*)\) im PATH \W+ .*?\. (.+?)\.?$", line)
    if match:
        return False, "gettext fehlt: " + match.group(1) + " – Installationshinweis: " + match.group(2)
    return None


def child_process_env(base=None):
    """Environment for build-quest.py under the builder: force UTF-8 on its pipes.

    Without it Windows writes SystemExit messages in the ANSI code page and the UTF-8 reader mangles them."""
    return {**(os.environ if base is None else base), "PYTHONIOENCODING": "utf-8"}


def detect_paths(root):
    sdk_candidates = [os.environ.get("ANDROID_HOME"), os.environ.get("ANDROID_SDK_ROOT"),
                      str(Path.home() / "Library/Android/sdk"), str(Path.home() / "Android/Sdk")]
    if os.environ.get("LOCALAPPDATA"):
        sdk_candidates.append(str(Path(os.environ["LOCALAPPDATA"]) / "Android/Sdk"))
    # Workspace-local toolchain, if installed alongside another XR project.
    adjacent = root.parent.parent / "XR-OpenRA/.toolchains"
    sdk_candidates.append(str(adjacent / "android-sdk"))
    java_candidates = [os.environ.get("JAVA_HOME"), str(adjacent / "jdk")]
    for directory in [Path("/Library/Java/JavaVirtualMachines"), Path.home() / "Library/Java/JavaVirtualMachines"]:
        if directory.is_dir():
            java_candidates += [str(p / "Contents/Home") for p in sorted(directory.glob("*17*"))]
    for directory in [Path("/usr/lib/jvm"), Path.home() / ".jdks"]:
        if directory.is_dir():
            java_candidates += [str(p) for p in sorted(directory.glob("*17*"))]
    for name in ["Eclipse Adoptium", "Microsoft", "Java"]:
        directory = Path(os.environ.get("ProgramFiles", "C:/Program Files")) / name
        if directory.is_dir():
            java_candidates += [str(p) for p in sorted(directory.glob("*17*"))]
    def first(candidates):
        return next((str(Path(p).expanduser()) for p in candidates if p and Path(p).expanduser().is_dir()), "")
    return first(sdk_candidates), first(java_candidates)
