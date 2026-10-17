#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
cd "$(dirname "$0")" || exit 1
if ! command -v python3 >/dev/null 2>&1; then
    echo "Python 3 mit Tkinter fehlt. Bitte Python von https://www.python.org/downloads/ installieren."
    read -r answer
    exit 1
fi
python3 scripts/quest-builder.py
result=$?
if [ "$result" -ne 0 ]; then
    echo "Start fehlgeschlagen. Siehe Meldung oben und docs/GUI-BUILDER.md. Enter zum Schließen."
    read -r answer
fi
exit "$result"
