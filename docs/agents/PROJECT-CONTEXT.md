# Projektkontext für GPT, Claude, Codex und andere Agenten

## Ziel und Stand
xr.rttr ist ein experimenteller Meta-Quest-3-Port von Return to the Roots. Das klassische Spiel wird auf einem verstellbaren OpenXR-Quad-Layer dargestellt; Controller ersetzen Maus/Tastatur. Projektspezifische Entwicklung: 100 % Vibe Coding mit Codex, Astra und Claude (Opus). Upstream-Code und Originalspieldaten haben eigene Urheber/Lizenzen.

Repository: https://github.com/friedensbringer-peacemaker/xr.rttr. Der Agent braucht lokalen Zugriff oder vom Nutzer bereitgestellte Dateien; ein Chatfenster hat nicht automatisch Zugriff auf den Rechner.

## Verzeichnisübersicht
| Pfad | Zweck |
| --- | --- |
| README.md | Einstieg, legale Datenquellen, Status |
| android/ | Gradle-/Android-App-Projekt |
| android/app/jni/s25client/ | Vollständiger angepasster Engine-Quellstand, kein separates Engine-Submodule |
| android/app/jni/s25client/extras/videoDrivers/OpenXR/ | Quest-Rendering, Controller-Aktionen, Panel-Geometrie |
| android/app/jni/s25client/libs/s25main/ingameWindows/iwVrTools.cpp | Gemeinsame VR-Tastatur für ctrlEdit und VR-Schnellmenü |
| android/app/jni/s25client/libs/s25main/desktops/dskGameInterface.cpp | Ingame-Eingabe und Y-Geschwindigkeitszyklus |
| android/external/ und Engine external/ | Festgelegte native Bibliotheken als Submodule |
| third_party/openxr-sdk-source/ | Festgelegter Khronos-Quellstand |
| scripts/build-quest.py | Preflight, Submodule, Loader-Build, Gradle-Build |
| scripts/builder_support.py | Gemeinsame Werkzeug-/Versionsprüfung und Pfaderkennung |
| scripts/quest-builder.py | Tkinter-GUI; Worker-Prozess, Queue, Protokoll, Abbruch |
| scripts/quest.py | ADB-Installation und geschützter Originaldatenimport |
| Quest-Builder.command / .cmd / .ps1 | Startvarianten für die GUI |
| docs/setup/ | Befehlsfolgen für Windows und macOS |
| docs/wiki/ | Projekt-/Nutzerdokumentation; Wiki-Seiten im Repository |
| docs/SOURCE-PROVENANCE.json | Herkunft und 88 direkte Submodule mit Commit-Ständen |
| docs/THIRD-PARTY.md | Lizenz- und Verteilhinweise |
| tests/test_builder.py | Offline-Preflight-Tests |
| .build/ | Ignorierte lokale Einstellungen, Loader-Build und GUI-Logs |

## Build-Vertrag
Python 3.9+ mit Tkinter für die GUI, Git, JDK 17, SDK Platform 36, Build-Tools 35.0.0, NDK 27.0.12077973, CMake 3.22.1. Gradle Wrapper 8.13 / Android-Plugin 8.13.2. ABI arm64-v8a. Keine Originaldaten zum Kompilieren erforderlich.

Zuerst `python3 scripts/build-quest.py --check --sdk <SDK> --java <JDK17>`, dann ohne `--check` bauen. Unter Windows Python-Aufruf an die Installation anpassen (`py -3.13`). Ausgabe: android/app/build/outputs/apk/debug/xr.rttr-Quest.apk. Unter Windows SDK/Java-Pfade nie ungequotet in Shelltexte einsetzen.

Der Loader wird aus Quellen nach android/app/src/main/jniLibs/arm64-v8a kopiert; die Binärdatei wird nicht committet. Gradle baut die übrigen nativen Bibliotheken und Engine. Ein GitHub-Quellcode-ZIP ist ohne Submodule kein vollständiger Checkout.

## Geräte-Vertrag
Paket (applicationId): xr.rttr, Java-Namespace bleibt org.s25rttr.sdl. Einstieg: xr.rttr/org.s25rttr.sdl.GameStartActivity. Originaldaten: /sdcard/RTTR/S2/DATA und GFX. Benutzerdaten: /sdcard/RTTR/.s25rttr. Gerätescreenshots: /sdcard/Oculus/Screenshots. Originaldaten, persönliche Spielstände und Geräteprotokolle nicht veröffentlichen. Screenshots nur ausgewählt und verkleinert (max. 1280 px Breite) unter `docs/images/` zur Projektdarstellung, nach dem Vorbild von siedler25.org mit Hinweis auf die Rechteinhaber am Ende der README; keine Aufnahmen mit Passthrough-Zimmer, Personen oder privaten Bildschirmen, keine Originaldateien oder Grafik-Extrakte.

`quest.py install` installiert mit `adb install -r` und startet regulär. `quest.py import-data <Ordner>` verlangt DATA/GFX und lehnt vorhandene Zielordner standardmäßig ab. `--replace-data` ist explizites Überschreiben, kein Kompatibilitätstest. Mehrere Geräte: `--serial` verwenden.

## Grenzen und Validierung
Vollständiger nativer Build auf macOS erfolgreich; GUI-Preflight, Fehlerfall mit alter APK, Abbruch und echter GUI-Build wurden auf macOS geprüft. CLI-Build unter Windows 10 am 27.09.2026 erfolgreich (gettext und Python im PATH nötig, siehe docs/BUILD.md); Windows-GUI und Linux nicht praktisch getestet. Maßgeblicher Arbeitsstand ist `main` (seit PR #1 identisch mit dem VR-Baseline-Neuaufbau) – vor Builds trotzdem Branches prüfen, vor Headset-Tests installierte `versionName` prüfen.

Startverhalten (preview.4): `Settings` lädt `presentation` immer als Leinwand; der OpenXR-Treiber zentriert vor der Kopfblickrichtung, sobald die Kopfpose gültig ist (außerhalb des Fokusblocks) und stellt beim Start nur Breite/Abstand wieder her. Optionales Trigger-Greifen: `VrSettings.triggerGrab`, Gestenlogik `vrInput::MapGrab`, Callbacks `VrMapAt`/`VrDrag` (Treiber-ABI 20), Spielseite `dskGameInterface::VrDrag` über `ViewPosToMap`. Private APK-Vorabversion vorhanden, jedoch keine erneute Headset-Abnahme des neuen vollständigen Repository-Builds. Aktuellste Details in docs/VALIDATION.md.

Der GUI-Builder baut APKs. Er ist noch kein automatischer SDK-Installer, Meta-Konto-Assistent oder GOG-Installer-Extractor. Der weitergehende Installationsassistent ist geplant. Den Unterschied in Hilfestellungen beibehalten.

## VR-Einstellungen
`VrSettings.h` definiert die gemeinsamen Werte, `Settings.video.vr` speichert sie. `Settings.vrPreview` bleibt flüchtig. `iwVrSettings` (in iwSettings.cpp) ist aus dskOptions und iwSettings erreichbar. WindowManager liefert die aktuellen Werte über den Treiber-Callback; Treiber-ABI ist Version 19. OpenXR wendet die Bildkorrektur nur beim Kopieren ins Panel an. PanelGeometry.h berechnet Treffer für flache/gebogene Flächen, passend zur optionalen Cylinder-Extension. Maus-Icon-Skalierung liegt in WindowManager::DrawCursor. Bei Änderungen alle Treiber zusammen bauen.

VR-Defaults entsprechen dem Nutzerprofil (80 % Helligkeit, 120 % Kontrast usw.; siehe VR-Optionen.md). Beim Laden fehlender Werte stets `VrSettings` als gemeinsame Quelle verwenden. `cursorSize == 4` blendet das Maus-Icon aus. Der OpenXR-Treiber bietet echte Ultrawide-Modi; bei Auflösungswechsel müssen Offscreen-FBO und Panel-Swapchain dieselbe neue Größe erhalten. Alte Ressourcen erst nach erfolgreicher Neuanlage ersetzen.

## VR-Komfort
`iwVrTools.cpp` enthält Tastatur, Favoriten-/Schnellmenü, Komfortseiten, Pause-Rückfrage und Auflösungsbestätigung. `VrInput.h` kapselt getestete Kurz-/Langdrucklogik, Glättung und Countdown. `VideoDriverWrapper` hält unbestätigte Auflösungen vor und stellt nach 15 s zurück; `Settings::Save` persistiert währenddessen den vorherigen Modus. Treiber-ABI ist 19. Controller-/Komfortwerte liegen in `VrSettings`. Praktische Abnahme ist separat zu dokumentieren.

## Ansichten / Diorama
`VrSettings.presentation` wählt Leinwand (0) oder Tischansicht (1, weiterhin 2D). `tableTilt`, `tableLoweringCm`, `tableDistance` werden getrennt gespeichert. `PanelGeometry::Pose` kippt um die Bildmitte und liefert exakt dieselbe Transformation für Compositor und Eingabe. Aktions-ID 17 wechselt die Ansicht; Standardbelegungen bleiben unverändert. Komfortseite 4 enthält die Auswahl. Siehe `docs/wiki/Diorama-und-Ansichten.md` für den geplanten separaten Gelände-/Spritepfad; dieser ist noch nicht implementiert. `tests/test_vr_views.cpp` prüft Pose, Quaternion und Controller-Treffer.

## Sticknavigation und Speichername
`VrInput.h` enthält `StickTurbo` und `PanAccumulator`. OpenXR liefert über `VrPan` zeitabhängige Pixelbewegungen; WindowManager blockiert modale Fenster und Ziele über Spielfenstern. `VrScroll` kann Listentreffer prüfen, ohne sofort zu scrollen. Der Griff der Nicht-Zeigerhand gibt standardmäßig den linken Stick frei. Rechter Stickdruck ist zusätzlich gebunden und dient nur als Turbo. ABI 12 wegen erweiterter Callbacks. `SaveVrPanel` speichert auch getrennte Abstände. Startzentrierung wartet auf gültiges Kopftracking; neutrale Sticks verändern die Breite nicht. `iwSave` schlägt optional Zeitstempel-Spieler-Karte vor, `SaveName.h` bereinigt und begrenzt Namensbestandteile UTF-8-sicher. `test_vr_navigation.cpp` deckt Tempo, Gesten, Abstände und Namensbestandteile ab.

## Versions- und Aufgabenpflege

Aktueller Entwicklungsstand: `main`. Der schrittweise Wiederaufbau aus `fix/vr-baseline-20260926` wurde am 27.09.2026 per PR #1 übernommen; die älteren experimentellen `main`-Commits (separate Menü-Layer usw.) sind nur noch Historie und wurden inhaltlich nicht übernommen. [Backlog](../../BACKLOG.md), [Updatelog](../../CHANGELOG.md) und [version.properties](../../version.properties) sind die zentralen Pflegeorte. Die APK liest Name und Code aus dieser Versionsdatei. Praktische Quest-Abnahme der neuesten Grip-Korrekturen ist noch offen.

## preview.3 Gebäudehilfe und HQ-Beta

`driver/BuildingHelp.h` enthält die vier festen HTTPS-Ziele; `iwHelp(content, true)` bietet den optionalen Browserbutton nur für Gebäudeinformationen. `VideoOpenXR::OpenBuildingHelp` nutzt SDL_OpenURL / Android ACTION_VIEW. Auswahl `buildingHelpSite` 0–4, Standard 0.

`headquarters3d` ist standardmäßig falsch; persistiert separat als `vr_headquarters3d_beta`, damit alte Experimente nicht automatisch aktiv werden. `nobHQ::Draw` fragt den Treiber vor dem Spritezeichnen. `HqAsset.h` lädt nur die externe `.xrhq`-Datei; keine Modelldaten im Repository. Modellposition basiert auf aktuellen Engine-Matrizen. Nur Tisch, vollständig sichtbares HQ und Spiel ohne Fenster; fehlendes Asset behält den Sprite. Stereo-Projektionslayer mit eigenen nativen Tiefen-Renderbuffern. Frühere GUI-Layer und Griffrotation bleiben entfernt. Hardware-Abnahme ausstehend.
