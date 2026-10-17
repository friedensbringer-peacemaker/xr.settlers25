@echo off
rem SPDX-License-Identifier: GPL-2.0-or-later
cd /d "%~dp0"
where py >nul 2>nul
if not errorlevel 1 (
    py -3 scripts\quest-builder.py
) else (
    python scripts\quest-builder.py
)
if errorlevel 1 (
    echo Start fehlgeschlagen. Python 3 mit Tkinter von python.org installieren.
    echo Weitere Hilfe: docs\GUI-BUILDER.md
    pause
)
