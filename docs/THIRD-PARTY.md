# Lizenzen und Herkunft

Dieses Repository enthält weiterverteilbaren Open-Source-Code und RTTR-eigene Ressourcen mit ihren Lizenzangaben. Es enthält keine kommerziellen Siedler-II-Originaldaten. „Open Source“ hebt das Urheberrecht nicht auf.

## Übernommene Quellstände
Die genauen Herkunfts-URLs und Commit-Stände sind in [SOURCE-PROVENANCE.json](SOURCE-PROVENANCE.json) dokumentiert. Die Android- und Engine-Verzeichnisse sind Snapshots der nach Quest portierten Arbeitsstände inklusive der jüngsten Änderungen. Historien der ursprünglichen Repositories wurden nicht in dieses Repository importiert.

| Bestandteil | Lizenzhinweise |
| --- | --- |
| Android-Port | `android/LICENSE` (GPL v2); ursprüngliche Hinweise in den Dateien erhalten |
| RTTR-Engine | `android/app/jni/s25client/LICENSE`, `LICENSES/` und SPDX-Hinweise; u. a. GPL-2.0-or-later |
| RTTR-eigene Grafiken | `android/app/jni/s25client/.reuse/dep5`: `data/RTTR/assets/*` unter GPL-2.0-or-later |
| RTTR-Musik unter `MUSIC/SNG` | Laut derselben Herkunftsdatei CC0-1.0 |
| OpenXR-Header | Khronos-Copyright-/SPDX-Hinweise in den Headern; Apache-2.0-Text unter `android/LICENSES/` |
| OpenXR-Loader | Aus festgelegtem Khronos-SDK-Source-Submodule gebaut; dortige LICENSE-/COPYING-/SPDX-Dateien beachten |
| SDL, SDL_mixer, Boost, Lua, gl4es, Bzip2, miniupnpc und Engine-Bibliotheken | Als festgelegte Submodule eingebunden; Lizenztexte bleiben jeweils im Originalquellbaum |

Neu hinzugefügte Build-/Übertragungsskripte werden unter GPL-2.0-or-later bereitgestellt; siehe LICENSE im Repository-Wurzelverzeichnis. Vor einem späteren öffentlichen Binärrelease müssen Lizenztexte, Hinweise und zugehöriger Quellstand gemeinsam bereitgestellt werden.

## Bewusst nicht enthalten
- Originale Siedler-II-Verzeichnisse `DATA` und `GFX`, GOG-Installer oder Datenträgerabbilder.
- Screenshots mit Originalgrafiken, private Spielstände und Geräteprotokolle.
- Signierschlüssel, Passwörter, lokale SDK-Pfade und Zugangsdaten.
- Generierte APKs, native Bibliotheken und Build-Caches. Der OpenXR-Loader wird aus den enthaltenen Referenzen neu gebaut.

Die `.gitignore` hilft gegen versehentliches Hinzufügen, ersetzt aber keine Kontrolle vor Commits. Die Submodule führen eigene Historien und Lizenzen; einige Upstream-Quellen liegen außerhalb von GitHub.
