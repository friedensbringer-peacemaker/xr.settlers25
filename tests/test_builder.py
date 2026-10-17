# SPDX-License-Identifier: GPL-2.0-or-later
"""Offline tests for toolchain validation; no SDK installation or device required."""
import os
import subprocess
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch
from types import SimpleNamespace

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
from builder_support import check_gettext, child_process_env, gettext_status, prepare_environment, validate_environment


class PreflightTests(unittest.TestCase):
    def test_selected_paths_override_old_environment_and_accept_mac_bundle(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory).resolve()
            java = root / "Java 17.jdk"
            (java / "Contents/Home").mkdir(parents=True)
            with patch.dict(os.environ, {"ANDROID_HOME": "old-sdk", "ANDROID_SDK_ROOT": "other-sdk", "JAVA_HOME": "old-java"}):
                env = prepare_environment(root / "SDK with spaces", java)
            self.assertEqual(env["ANDROID_HOME"], str(root / "SDK with spaces"))
            self.assertEqual(env["ANDROID_SDK_ROOT"], env["ANDROID_HOME"])
            self.assertEqual(env["JAVA_HOME"], str(java / "Contents/Home"))
            self.assertEqual(env["PATH"].split(os.pathsep)[0], str(java / "Contents/Home/bin"))

    def test_missing_tools_report_actionable_errors(self):
        with tempfile.TemporaryDirectory() as directory, patch("builder_support.shutil.which", return_value=None):
            errors, _ = validate_environment(Path(directory), {"PATH": ""})
        self.assertTrue(any("Git fehlt" in e for e in errors))
        self.assertTrue(any("JDK-17" in e for e in errors))
        self.assertTrue(any("Android-SDK" in e for e in errors))

    def test_wrong_java_version_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory).resolve()
            (root / ".git").mkdir()
            (root / "bin").mkdir()
            for name in ("java", "javac"):
                (root / "bin" / (name + (".exe" if os.name == "nt" else ""))).touch()
            with patch("builder_support.shutil.which", return_value="git"), patch("builder_support.subprocess.run", return_value=SimpleNamespace(returncode=0, stdout='', stderr='openjdk version "21.0.1"')):
                errors, _ = validate_environment(root, {"JAVA_HOME": str(root), "ANDROID_HOME": str(root)})
            self.assertTrue(any("JDK 17 auswählen" in e for e in errors))
            self.assertTrue(any("SDK-Lizenz" in e for e in errors))

    def test_gettext_present_is_reported_ok(self):
        with patch("builder_support.shutil.which", return_value="msgfmt") as which:
            errors, messages = check_gettext({"PATH": "tools"})
        self.assertEqual(errors, [])
        self.assertTrue(any("OK: gettext" in m for m in messages))
        self.assertEqual({c.args[0] for c in which.call_args_list}, {"msgfmt", "msgmerge"})

    def test_gettext_missing_is_error_with_platform_hint(self):
        for platform, hint in (("win32", "winget"), ("darwin", "brew"), ("linux", "apt"), ("freebsd", "winget")):
            with self.subTest(platform=platform):
                with patch("builder_support.shutil.which", side_effect=lambda tool, path=None: None if tool == "msgfmt" else tool):
                    errors, messages = check_gettext({"PATH": "C:/Users/secret/bin"}, platform)
                self.assertEqual(messages, [])
                self.assertEqual(len(errors), 1)
                self.assertIn("msgfmt", errors[0])
                self.assertNotIn("msgmerge", errors[0])
                self.assertIn(hint, errors[0])
                self.assertNotIn("secret", errors[0])

    def test_gettext_status_row_ok(self):
        with patch("builder_support.shutil.which", return_value="msgfmt"):
            _, messages = check_gettext({"PATH": "tools"})
        self.assertEqual(gettext_status(messages[0] + "\n"), (True, "gettext: OK"))
        self.assertIsNone(gettext_status("OK: Android SDK\n"))

    def test_gettext_status_row_missing_has_hint_without_paths(self):
        for platform, hint in (("win32", "winget install --id mlocati.GetText"), ("darwin", "brew install gettext"),
                               ("linux", "sudo apt install gettext"), ("freebsd", "brew install gettext")):
            with self.subTest(platform=platform):
                with patch("builder_support.shutil.which", side_effect=lambda tool, path=None: None if tool == "msgfmt" else tool):
                    errors, _ = check_gettext({"PATH": "C:/Users/secret/bin"}, platform)
                ok, text = gettext_status(errors[0])
                self.assertFalse(ok)
                self.assertTrue(text.startswith("gettext fehlt: msgfmt – Installationshinweis: "))
                self.assertIn(hint, text)
                self.assertNotIn("secret", text)
                self.assertNotIn("PATH", text)

    def test_gettext_status_row_missing_across_process_boundary(self):
        # Same path as the builder: child raises SystemExit, parent reads the pipe as UTF-8.
        script = ("import sys; sys.path.insert(0, sys.argv[1]); from builder_support import check_gettext; "
                  "errors, _ = check_gettext({'PATH': ''}, 'win32'); raise SystemExit(errors[0])")
        base = {k: v for k, v in os.environ.items() if k not in ("PYTHONUTF8", "PYTHONIOENCODING")}
        result = subprocess.run([sys.executable, "-c", script, str(Path(__file__).resolve().parents[1] / "scripts")],
                                env=child_process_env(base), stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                text=True, encoding="utf-8", errors="replace", timeout=60)
        self.assertEqual(result.returncode, 1)
        self.assertNotIn("�", result.stdout)
        ok, text = gettext_status(result.stdout.strip().splitlines()[-1])
        self.assertFalse(ok)
        self.assertIn("winget install --id mlocati.GetText", text)

    def test_gettext_status_tolerates_mangled_separator(self):
        for dash in ("�", "?", "-", "–"):
            with self.subTest(dash=dash):
                line = "Fehlt: gettext (msgfmt, msgmerge) im PATH " + dash + " n�tig f�r X. Windows: winget install --id mlocati.GetText."
                ok, text = gettext_status(line)
                self.assertFalse(ok)
                self.assertEqual(text, "gettext fehlt: msgfmt, msgmerge – Installationshinweis: Windows: winget install --id mlocati.GetText")


if __name__ == "__main__":
    unittest.main()
