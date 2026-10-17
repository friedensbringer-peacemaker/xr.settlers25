#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Cross-platform Tkinter frontend for the Quest APK build."""
import json
import os
from pathlib import Path
import queue
import signal
import subprocess
import sys
import threading
import time
import webbrowser
try:
    import tkinter as tk
    from tkinter import filedialog, messagebox, scrolledtext, ttk
except ImportError:
    raise SystemExit("Tkinter fehlt. Windows/macOS: Python von python.org installieren; Linux: python3-tk installieren.")
from builder_support import child_process_env, detect_paths, gettext_status

ROOT = Path(__file__).resolve().parents[1]
APK = ROOT / "android/app/build/outputs/apk/debug/xr.rttr-Quest.apk"
SETTINGS = ROOT / ".build/builder-settings.json"


class Builder:
    def __init__(self, window):
        self.window = window
        self.events = queue.Queue()
        self.process = None
        self.busy = False
        self.cancelled = False
        self.close_requested = False
        self.started = 0
        self.mode = ""
        window.title("xr.rttr · Quest APK Builder")
        window.geometry("900x690")
        window.minsize(730, 570)
        window.protocol("WM_DELETE_WINDOW", self.close)
        sdk, java = detect_paths(ROOT)
        try:
            saved = json.loads(SETTINGS.read_text())
            sdk, java = saved.get("sdk", sdk), saved.get("java", java)
        except (OSError, ValueError, AttributeError):
            pass
        self.sdk = tk.StringVar(value=sdk)
        self.java = tk.StringVar(value=java)
        self.status = tk.StringVar(value="Bereit. Zuerst die Voraussetzungen prüfen.")
        self.elapsed = tk.StringVar(value="")
        body = ttk.Frame(window, padding=22)
        body.pack(fill="both", expand=True)
        ttk.Label(body, text="Quest-APK bauen", font=("", 22, "bold")).pack(anchor="w")
        ttk.Label(body, text="1  Werkzeuge auswählen     →     2  Prüfen     →     3  APK bauen",
                  padding=(0, 8, 0, 4)).pack(anchor="w")
        ttk.Label(body, text="Für den Build sind keine Siedler-Spieldaten und keine angeschlossene Quest nötig.",
                  wraplength=790).pack(anchor="w", pady=(0, 18))
        fields = ttk.Frame(body)
        fields.pack(fill="x")
        fields.columnconfigure(1, weight=1)
        self.inputs = []
        for row, (label, variable) in enumerate([("Android SDK", self.sdk), ("Java JDK 17", self.java)]):
            ttk.Label(fields, text=label, width=15).grid(row=row, column=0, sticky="w", pady=5)
            entry = ttk.Entry(fields, textvariable=variable)
            entry.grid(row=row, column=1, sticky="ew", padx=(0, 8), pady=5)
            button = ttk.Button(fields, text="Ordner wählen…", command=lambda v=variable: self.choose(v))
            button.grid(row=row, column=2, pady=5)
            self.inputs.extend([entry, button])
        links = ttk.Frame(body)
        links.pack(fill="x", pady=(8, 14))
        for text, url in [
            ("Java 17 laden", "https://adoptium.net/temurin/releases/?version=17"),
            ("Android Studio / SDK", "https://developer.android.com/studio"),
            ("Git laden", "https://git-scm.com/downloads"),
            ("Schritt-für-Schritt-Hilfe", "https://github.com/friedensbringer-peacemaker/xr.rttr/blob/main/docs/GUI-BUILDER.md"),
        ]:
            ttk.Button(links, text=text, command=lambda u=url: webbrowser.open(u)).pack(side="left", padx=(0, 8))
        buttons = ttk.Frame(body)
        buttons.pack(fill="x", pady=(0, 12))
        self.check = ttk.Button(buttons, text="Voraussetzungen prüfen", command=lambda: self.start(check=True))
        self.check.pack(side="left", padx=(0, 8))
        self.build = ttk.Button(buttons, text="APK bauen", command=self.start)
        self.build.pack(side="left", padx=(0, 8))
        self.cancel = ttk.Button(buttons, text="Abbrechen", command=self.stop, state="disabled")
        self.cancel.pack(side="left")
        self.open_apk = ttk.Button(buttons, text="APK-Ordner öffnen", command=self.open_output)
        self.open_apk.pack(side="right")
        self.progress = ttk.Progressbar(body, mode="indeterminate")
        self.progress.pack(fill="x", pady=(0, 8))
        ttk.Label(body, textvariable=self.status, wraplength=790).pack(anchor="w")
        self.gettext = ttk.Label(body, text="gettext: noch nicht geprüft", font=("", 12, "bold"), wraplength=790)
        self.gettext.pack(anchor="w", pady=(4, 0))
        ttk.Label(body, textvariable=self.elapsed).pack(anchor="w", pady=(2, 8))
        self.log = scrolledtext.ScrolledText(body, height=15, wrap="word", state="disabled", font=("Courier", 11))
        self.log.pack(fill="both", expand=True)
        ttk.Label(body, text="Der erste Build lädt Abhängigkeiten aus dem Internet und kann länger dauern.\n"
                  "Fehlende SDK-Komponenten und deren Lizenzen werden im SDK Manager eingerichtet.",
                  wraplength=790, padding=(0, 10, 0, 0)).pack(anchor="w")
        self.write("Projekt: " + str(ROOT) + "\n")
        self.refresh_output()
        window.after(100, self.poll)

    def write(self, text):
        self.log.configure(state="normal")
        self.log.insert("end", text)
        self.log.see("end")
        self.log.configure(state="disabled")

    def choose(self, variable):
        folder = filedialog.askdirectory(parent=self.window, title="Werkzeugordner auswählen")
        if folder:
            variable.set(folder)

    def refresh_output(self):
        self.open_apk.configure(state="normal" if APK.is_file() and not self.busy else "disabled")

    def start(self, check=False):
        if self.busy:
            return
        sdk, java = self.sdk.get().strip(), self.java.get().strip()
        if not sdk or not java:
            messagebox.showinfo("Ordner fehlen", "Bitte Android SDK und Java JDK 17 auswählen.", parent=self.window)
            return
        try:
            SETTINGS.parent.mkdir(parents=True, exist_ok=True)
            SETTINGS.write_text(json.dumps({"sdk": sdk, "java": java}, indent=2))
        except OSError as error:
            messagebox.showerror("Einstellungen", str(error), parent=self.window)
            return
        self.busy, self.cancelled = True, False
        self.started = time.monotonic()
        self.mode = "check" if check else "build"
        self.log.configure(state="normal")
        self.log.delete("1.0", "end")
        self.log.configure(state="disabled")
        self.status.set("Voraussetzungen werden geprüft…" if check else "APK-Build läuft – Protokoll siehe unten.")
        self.gettext.configure(text="gettext: wird geprüft…", foreground="")
        for control in self.inputs + [self.check, self.build]:
            control.configure(state="disabled")
        self.cancel.configure(state="normal")
        self.refresh_output()
        self.progress.start(12)
        command = [sys.executable, "-u", str(ROOT / "scripts/build-quest.py"), "--sdk", sdk, "--java", java]
        if check:
            command.append("--check")
        threading.Thread(target=self.worker, args=(command,), daemon=True).start()

    def worker(self, command):
        try:
            logfile = ROOT / ".build" / ("builder-" + time.strftime("%Y%m%d-%H%M%S") + ".log")
            kwargs = {"creationflags": subprocess.CREATE_NEW_PROCESS_GROUP} if os.name == "nt" else {"start_new_session": True}
            with logfile.open("w", encoding="utf-8") as output:
                process = subprocess.Popen(command, cwd=ROOT, env=child_process_env(), stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                           text=True, encoding="utf-8", errors="replace", bufsize=1, **kwargs)
                self.events.put(("process", process))
                self.events.put(("line", "Protokoll: " + str(logfile) + "\n"))
                for line in process.stdout:
                    output.write(line)
                    output.flush()
                    self.events.put(("line", line))
                code = process.wait()
                self.events.put(("done", code))
        except Exception as error:
            self.events.put(("line", "Fehler: " + str(error) + "\n"))
            self.events.put(("done", -1))

    def terminate(self, process):
        try:
            if os.name == "nt":
                subprocess.run(["taskkill", "/PID", str(process.pid), "/T", "/F"],
                               stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=15)
            else:
                os.killpg(process.pid, signal.SIGTERM)
                time.sleep(3)
                try:
                    os.killpg(process.pid, signal.SIGKILL)
                except ProcessLookupError:
                    pass
        except (OSError, subprocess.TimeoutExpired):
            pass

    def stop(self):
        if not self.busy:
            return
        self.cancelled = True
        self.status.set("Abbruch angefordert…")
        self.cancel.configure(state="disabled")
        if self.process:
            threading.Thread(target=self.terminate, args=(self.process,), daemon=True).start()

    def close(self):
        if self.busy:
            if messagebox.askyesno("Build läuft", "Vorgang abbrechen und Fenster schließen?", parent=self.window):
                self.close_requested = True
                self.stop()
        else:
            self.window.destroy()

    def open_output(self):
        folder = str(APK.parent)
        if os.name == "nt":
            os.startfile(folder)
        elif sys.platform == "darwin":
            subprocess.Popen(["open", folder])
        else:
            subprocess.Popen(["xdg-open", folder])

    def poll(self):
        # Bound work per tick so heavy compiler output cannot freeze the UI.
        for _ in range(250):
            try:
                kind, value = self.events.get_nowait()
            except queue.Empty:
                break
            if kind == "line":
                self.write(value)
                result = gettext_status(value)
                if result:
                    self.gettext.configure(text=result[1], foreground="#1a7f37" if result[0] else "#c62828")
            elif kind == "process":
                self.process = value
                if self.cancelled:
                    threading.Thread(target=self.terminate, args=(value,), daemon=True).start()
            elif kind == "done":
                self.busy = False
                if self.gettext.cget("text") == "gettext: wird geprüft…":
                    self.gettext.configure(text="gettext: nicht geprüft – Ursache im Protokoll")
                self.process = None
                self.progress.stop()
                self.cancel.configure(state="disabled")
                for control in self.inputs + [self.check, self.build]:
                    control.configure(state="normal")
                if self.cancelled:
                    self.status.set("Abgebrochen. Eine eventuell vorhandene ältere APK wurde nicht aktualisiert.")
                elif value == 0 and self.mode == "check":
                    self.status.set("Voraussetzungen erfüllt. Du kannst jetzt die APK bauen.")
                elif value == 0 and APK.is_file():
                    self.status.set("APK erfolgreich gebaut. Über „APK-Ordner öffnen“ findest du die Datei.")
                else:
                    self.status.set("Nicht erfolgreich. Ursache im Protokoll prüfen; eine ältere APK ist kein neues Ergebnis.")
                self.refresh_output()
                if self.close_requested:
                    self.window.destroy()
                    return
        if self.busy:
            seconds = int(time.monotonic() - self.started)
            self.elapsed.set(f"Laufzeit: {seconds // 60:02d}:{seconds % 60:02d}")
        self.window.after(100, self.poll)


if __name__ == "__main__":
    window = tk.Tk()
    Builder(window)
    window.mainloop()
