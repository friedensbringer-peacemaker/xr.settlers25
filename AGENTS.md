# xr.rttr – Arbeitskontext für Coding-Agenten

Lies zuerst README.md und docs/agents/PROJECT-CONTEXT.md. Für Hilfe bei Einrichtung/Installation verwende docs/agents/SETUP-RUNBOOK.md; kopierbare Nutzer-Prompts stehen in docs/agents/START-PROMPTS.md.

- Dieses Repository ist die maßgebliche Quelle. Android-Projekt: android/. Eingebettete Engine: android/app/jni/s25client/. Keine zweite Engine-Kopie pflegen.
- Externe Quellen sind auf Commit-Stände festgelegte Submodule. Nicht pauschal aktualisieren. Herkunft und Lizenzen erhalten.
- Keine Originalspieldaten, persönlichen Spielstände, Signierschlüssel, SDKs oder Zugangsdaten committen. Keine Zugangsdaten in Ausgaben zeigen.
- GUI und CLI teilen scripts/builder_support.py und scripts/build-quest.py. Nutzerpfade lokal unter .build/ bzw. android/local.properties halten.
- Prüfen: python3 -m unittest discover -s tests -v; python3 scripts/build-quest.py --check. Für GUI-Änderungen auch Tkinter-Laufzeit/Abbruch prüfen.
- Bestehende Benutzeränderungen erhalten. Bei Geräteschritten Zustand und Seriennummer prüfen; nicht automatisch deinstallieren oder Benutzerdateien löschen. Vor Updates laufendes Spiel speichern lassen; vorhandene Freigaben der aktuellen Sitzung berücksichtigen.
- Regulärer Start: xr.rttr/org.s25rttr.sdl.GameStartActivity, niemals direkter Kaltstart der SDLActivity.
- Behauptete Plattform-/Headset-Tests müssen tatsächlich durchgeführt sein. Windows/Linux sind bisher nicht praktisch abgenommen.

- Projektpflege: BACKLOG.md und CHANGELOG.md aktuell halten. Version ausschließlich in version.properties ändern; für jede verteilte APK versionCode erhöhen. Gebaut, installiert und praktisch auf Quest bestätigt unterscheiden.
