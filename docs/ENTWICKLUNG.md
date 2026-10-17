# Entwicklung

Technische Informationen zum Repository. Für Spieler reicht die [Startseite](../README.md).

## Was liegt in diesem Repository?

- `android/`: Android-Projekt und vollständiger angepasster RTTR-Engine-Quellstand unter `app/jni/s25client/`.
- Externe Open-Source-Bibliotheken als **Submodule mit festen Commit-Ständen**; ihre Quellen werden beim rekursiven Klonen aus den Original-Repositories geladen. Bzip2 und einige verschachtelte Abhängigkeiten liegen bei ihren jeweiligen Upstream-Hostern, nicht alle bei GitHub.
- `third_party/openxr-sdk-source/`: festgelegter Khronos-Quellstand zum Bauen des OpenXR-Loaders.
- `scripts/`: Build, ADB-Installation und Übertragung eigener Originaldaten.
- `docs/`: Bedienung, Build-Anleitung, Kompatibilität und Planung des Einrichtungsassistenten. Die Wiki-Seiten liegen unter `docs/wiki`.

Originalspieldaten, persönliche Spielstände, SDK/NDK/JDK, Zugangsdaten, Signierschlüssel und Build-Caches werden nicht mitgeliefert.

## Stand und Planung

- [Updatelog](../CHANGELOG.md) – was sich in welcher Version geändert hat
- [Backlog](../BACKLOG.md) – geplante und offene Aufgaben
- [Prüfbericht](VALIDATION.md) – was tatsächlich gebaut und im Headset getestet wurde
- [version.properties](../version.properties) – zentrale Versionsquelle der APK (Paket `xr.rttr`)

## APK bauen

**Mit Oberfläche:** `Quest-Builder.command` (macOS) bzw. `Quest-Builder.cmd` (Windows) starten, unter Linux `python3 scripts/quest-builder.py`. Java und Android SDK auswählen, Voraussetzungen prüfen, **APK bauen**. [Anleitung](GUI-BUILDER.md)

**Im Terminal:**

```sh
git clone --recurse-submodules https://github.com/friedensbringer-peacemaker/xr.rttr.git
cd xr.rttr
python3 scripts/build-quest.py
```

Voraussetzungen: **[Build-Anleitung](BUILD.md)**. Unter Windows `python` oder `py -3` verwenden; zusätzlich muss gettext (`msgfmt`) im PATH liegen. Ein ZIP-Download enthält die Submodule nicht vollständig; zum Klonen des privaten Repositorys ist Zugriff erforderlich.

- [Windows: alle Schritte in PowerShell](setup/Windows-PowerShell.md)
- [macOS: alle Schritte im Terminal](setup/macOS-Terminal.md)

## KI-Agenten

Codex und andere lokale Agenten finden den Einstieg in `AGENTS.md`; für Claude verweist `CLAUDE.md` auf dieselben Unterlagen: [Projektaufbau](agents/PROJECT-CONTEXT.md), [Einrichtungs-Runbook](agents/SETUP-RUNBOOK.md), [Start-Prompts](agents/START-PROMPTS.md). Reinen Chat-Assistenten müssen die Dateien bereitgestellt werden.
