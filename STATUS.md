# Status xr.rttr

Zuletzt aktualisiert: 2026-09-30 · Gerät: Meta Quest 3 (Horizon-OS-Version nicht erfasst)

**Geprüft** heißt: im Headset beobachtet (Screenshot, Video oder Rückmeldung des Nutzers) – nicht nur gebaut oder
per Log bestätigt. Alle Angaben unten stammen aus [CHANGELOG.md](CHANGELOG.md) und
[docs/VALIDATION.md](docs/VALIDATION.md); was dort nicht belegt ist, steht hier als „nicht belegt“.

## Aktueller Stand

| Feld | Wert |
|---|---|
| Version ([version.properties](version.properties)) | `2.1.0-preview.13`, `versionCode=15` |
| App-ID / Anzeigename | `xr.rttr` |
| Regulärer Start | `xr.rttr/org.s25rttr.sdl.GameStartActivity` (nie die SDLActivity direkt kalt starten) |
| Spieldaten auf dem Gerät | `/sdcard/RTTR/S2/DATA` und `GFX`; Benutzerdaten `/sdcard/RTTR/.s25rttr` |

## Rückroll-Punkte (Git-Tags)

| Tag | Commit | Inhalt | Zustand |
|---|---|---|---|
| `v2.1.0-preview.8` | `25b6d5f` | Screenshot-Modus im VR-Schnellmenü, Stand inkl. Tisch-Simulator, Scrolltempo, 4×-Zoom | vom Nutzer als funktionierender Stand freigegeben und als GitHub-Vorabversion veröffentlicht; **letzter freigegebener Stand** |

Die älteren `quest-preview-2026-09-26-*`-Tags stammen aus dem Neuaufbau vor PR #1 und sind keine freigegebenen Stände.

## Previews preview.8 bis preview.13

| Version | Code | Gebaut | Installiert | Im Headset bestätigt |
|---|---|---|---|---|
| 2.1.0-preview.8 | 10 | ja | ja, 27.09.2026 22:58 (121 Benutzerdateien vorher gesichert) | teilweise: vom Nutzer als funktionierender Stand freigegeben; Einzelabnahme von Screenshot-Modus, Tisch-Simulator, Scrolltempo und 4×-Zoom offen |
| 2.1.0-preview.9 | 11 | ja; `test_vr_realtable` und `test_vr_navigation` auf der Quest als ARM64-Programm bestanden | ja, 27.09.2026 23:32 (121 Benutzerdateien vorher gesichert) | nein – Tischerkennung und Lage offen |
| 2.1.0-preview.10 | 12 | ja | unklar: laut CHANGELOG „noch nicht auf die Quest übertragen“, die in preview.11 gemeldete Start-Schleife wird aber preview.10 zugeordnet (setzt eine Installation voraus); kein Installationsprotokoll | nein |
| 2.1.0-preview.11 | 13 | ja | nein („noch nicht auf die Quest übertragen“) | nein – Ursache der Start-Schleife per Headset-Protokoll zu bestätigen |
| 2.1.0-preview.12 | 14 | ja | nein („noch nicht auf die Quest übertragen“) | nein |
| 2.1.0-preview.13 | 15 | ja | ja (Smoke-Test, Brille im Ruhezustand): regulärer Start ohne Absturz, ANR und Start-Schleife, Prozess > 1 min stabil | nein – nur Smoke-Test ohne aufgesetzte Brille |

versionCode je Preview aus der Git-Historie von version.properties (Commits `4c5f533` … `f63c33f`).

## Gebaut, noch nicht geprüft

- Neustart nach Absturz leitet auf den regulären Start um (preview.10, Korrektur preview.11/.13) – simulierter Absturz-Neustart wurde von der Quest mit „Controller erforderlich“ abgefangen, mit aufgesetzter Brille zu wiederholen.
- Absturz beim Öffnen des Tisch-Simulators behoben (preview.10) – Öffnen im Headset prüfen.
- Anzeigename `xr.rttr` (preview.10) – Launcher, „Unbekannte Quellen“, Systemleiste prüfen.
- Echter Tisch aus dem Raumscan (preview.9, Ausschalten korrigiert in preview.13).
- Video-Aufnahme von Spielbild und Ton (preview.12, Neustart-Schutz in preview.13).
- Screenshot-Modus (preview.8), Tisch-Simulator (preview.7), Scrolltempo und 4×-Zoom (preview.6).

## Offene Headset-Abnahmen (BACKLOG-IDs)

Status laut [BACKLOG.md](BACKLOG.md) „Implementiert/Beta, Quest-Abnahme offen“:

- VR-001, VR-002, VR-003 – Zoom per linkem Grip, Kartenziehen per rechtem Grip, Fenster am Rahmen verschieben
- VR-004 – Achsen tauschen/umkehren je Controller
- VR-005 – Ladebalken und Geduldshinweis
- VR-011 – Passthrough-Schalter und Regler
- VR-012 – externes HQ-Modell (Beta)
- VR-015 – kein Zeiger/Strahl bei Splash, Intro und Kartenladen
- VR-016 – 40-Pixel-Raster der Menüs
- VR-017 – langer rechter Meta-Druck setzt Raumposition zurück
- VR-018 – Tischmitte 0–90 cm
- VR-019 – Gebäudehilfe im Browser
- VR-020 – zentrierter Leinwandstart, Trigger-Greifen
- VR-021 – Tisch-Simulator
- VR-022 – echter Tisch (Stufe 1+2)

Ohne eigene BACKLOG-ID offen: Screenshot-Modus, Scrolltempo/4×-Zoom, Video-Aufnahme, Absturz-Neustart,
Anzeigename in der Systemleiste.

## Bekannt offen

- Das Systemfenster der Quest („App-Name nicht verfügbar / Beenden“) gehört zur Meta-Oberfläche und kann von der App nicht unterdrückt werden (CHANGELOG preview.6/preview.8).
- Windows-GUI-Builder und Linux-Build nicht praktisch getestet (SETUP-001).
