# Updatelog / Changelog

Dieses Protokoll beschreibt den Quest-Port, nicht die Upstream-Version von Return to the Roots. Zentrale Versionsquelle: [version.properties](version.properties). Geplante Arbeit: [BACKLOG.md](BACKLOG.md).

## Unveröffentlicht

Neue Änderungen nach dem nächsten versionierten Prüfstand hier sammeln.

- **Öffentlicher Name jetzt `xr.rttr`** (Repository, App-ID `applicationId` und Anzeigename im öffentlichen Stand). Ältere Einträge unten nennen den Namen einheitlich `xr.rttr`; der Entwicklungsstand lief bis hierhin unter einer früheren App-ID. Ein App-ID-Wechsel bedeutet auf der Quest eine neue, getrennte App: Spielstände und Einstellungen müssen umgezogen werden.
- Öffentlicher Stand ohne Screenshots mit Originalgrafik und ohne interne Studio-Planungsunterlagen; neutrale Formulierungen in README und Doku.
  - Nachweis: nur Doku/Bezeichner geändert, nicht gebaut, nicht im Headset abgenommen.

## 2.1.0-preview.14 — 2026-10-04

Enthält den VR-Menü-Pflichtumfang (Merge `33d685b`) und die übrigen bisher unveröffentlichten Einträge darunter.

- **Zeiger-Versatz X/Y (VR-Menü-Pflicht P3):** Im Reiter „Strahl / Zeiger“ der VR-Einstellungen neue Zeilen **„Zeiger waagerecht“** und **„Zeiger senkrecht“** – Regler mit ‹ › (0,25 % je Schritt, ±15 %), Anzeige „0“, „+1,25 %“, „-0,50 %“ (ASCII-Minus, die Spielschrift kennt kein typografisches). Der Versatz wird im OpenXR-Treiber auf die Strahl-Bildkoordinate angewendet, bevor sie auf Spielkoordinaten abgebildet wird (`uv' = clamp(uv + p/100, 0, 1)`); Strahl und Zielpunkt bleiben, wo sie sind, nur der Spielzeiger rückt. In den VR-Menüs selbst (`vrMenuFocus`) wirkt er nicht. Gespeichert als `vr_ptrOffsetX`/`vr_ptrOffsetY` in Viertelprozent (-60..60), beim Laden geklemmt; Logik `vrMenuModel::ptroff` nach Spezifikation `xrc/menu.h` (TODO: nach Tag ersetzen). Treiber-ABI 26. Dauerdruck auf ‹ › selbst wiederholt nicht (Engine-Regler); Halten wiederholt über den Stick-Fokus.

- **VR-Menü-Pflichtumfang (VR-024, Zweig `xr-menu-pflicht`):** Umsetzung des verbindlichen VR-Menü-Umfangs (Skill xr-vr-menu §0).
  - *Beenden (P3):* Neuer roter Knopf „Spiel beenden“ direkt im VR-Schnellmenü. Erster Klick schaltet ihn sichtbar scharf (anderes Rot, beleuchtet, gelbe Schrift „Sicher? Nochmal klicken (3 s)“ mit Restzeit, Doppelpuls am Zeiger-Controller), zweiter Klick innerhalb 3 s speichert die Einstellungen und – im laufenden Spiel, nicht im Replay – den Auto-Speicherstand (`GameClient::SaveAutosaveNow`, gleicher Dateiname wie das periodische Auto-Save) und beendet dann wie „Beenden“ im Hauptmenü. Nach 3 s, bei jeder anderen Aktion im Fenster oder beim Schließen fällt der Knopf zurück.
  - *Strahl/Zielpunkt Aus (P3/P4):* „Strahl“ (Dünn/Mittel/Dick/**Aus**) und „Zielpunkt“ (Punkt/Ring/Kreuz/**Aus**) im Reiter „Strahl / Zeiger“; gespeichert getrennt als `vr_rayVisible`/`vr_markerVisible`, Stärke/Form bleiben erhalten. „Aus“ gilt nur auf der freien Spielkarte; in Fenstern und Menü-Desktops bleiben Strahl und Zielpunkt sichtbar. „Maus-Icon Aus“ unverändert.
  - *Zwei Stufen (P5):* Schnellmenü = 6 Favoriten + genau 8 große Knöpfe (Weiter, Fenster zentrieren, Leinwand/Tisch, Passthrough, VR-Einstellungen, Spiel-Einstellungen, Werkzeuge / alle Aktionen, Spiel beenden). GUI-Größe, Tisch-Simulator, Screenshot-Modus, Video und Passthrough-Stärke auf der zweiten Seite; „VR-Komfort“ nur noch über VR-Einstellungen → „Weitere VR-Optionen“. „Zentrieren“ (vorher 3×) und „Leinwand/Tisch“ (vorher 2×) gibt es nur noch im Schnellmenü. Passthrough-Knopf ausgegraut, wenn die Laufzeit kein Passthrough bietet.
  - *Stick/A/B (P2):* In allen VR-Fenstern bewegt der rechte Stick einen gelben Fokusrahmen (sofort, Halten wiederholt nach 450 ms alle 200 ms), A löst den Knopf aus bzw. stellt eine Auswahlliste/den Regler eine Stufe weiter (links/rechts ebenso), B schließt das Fenster. Die erste Stick-/A-Betätigung zeigt nur den Fokus. Belegte Spielaktionen auf A/B ruhen, solange ein VR-Fenster oben liegt, und wirken erst nach dem Loslassen wieder.
  - Logik in `driver/VrMenuModel.h` (abhängigkeitsfrei, nach Spezifikation `xrc/menu.h`; TODO: nach Tag von xr-common ersetzen); Treiber-ABI 25 (neue KeyTypes, `VrSettings`-Felder, `VrPassthroughAvailable`).
  - Bewusst ausgelassen: Zusammenlegen von „VR: Controller / Bild“ und „VR-Komfort“ zu einem Reiterfenster und der Verzicht auf den Fenster-Neuaufbau je Reiter in `iwVrComfort` (Control-IDs überschneiden sich je Seite, Entwurf/Vorschau-Logik müsste umgebaut werden – nicht risikoarm ohne Headset).
  - Nachweis: gebaut (Windows 10, `build-quest.py`, BUILD SUCCESSFUL, APK `xr.rttr-Quest.apk` 2.1.0-preview.13/versionCode 15 unverändert, da nicht verteilt; geänderte Engine-/Treiberdateien mit `-Wall -Wextra` ohne Warnung; APK-Gate meldet unverändert die schon vorher fehlenden Manifest-Einträge OpenXR-Permission/IMMERSIVE_HMD/Runtime-Query, Manifest nicht angefasst); Host-Test `tests/test_vr_menu.cpp` 213 Prüfungen grün (Beenden-Zustandsmaschine über 6 Bildraten × 7 Klickzeitpunkte, ungültige Zeitschritte, Abbruch, Sichtbarkeits-Wahrheitstabelle, Doppelpuls, Stick-Wiederholung, Fokus-Raster), bestehende Host-Tests (comfort, navigation, views, panel) weiter grün; nicht auf die Quest übertragen, nicht im Headset abgenommen.

- **Build-Prüfung (SETUP-001):** `build-quest.py --check` (und damit die Prüfung im grafischen Builder) meldet jetzt, ob die gettext-Werkzeuge `msgfmt` und `msgmerge` im PATH liegen. Fehlen sie, ist das ein **Fehler**, keine Warnung: `s25client/CMakeLists.txt` verlangt `find_package(Gettext REQUIRED)` für die Übersetzungen, der Engine-Build bricht sonst ab. Die Meldung nennt das fehlende Werkzeug und den Installationsweg für das eigene System (Windows `winget install --id mlocati.GetText`, macOS `brew install gettext`, Linux Paket `gettext`), ohne Pfade auszugeben. Gebaut: 5 Python-Tests bestanden (2 neu: vorhanden / fehlend je Plattform), `--check` auf dem Windows-10-Rechner durchgelaufen (`OK: gettext (msgfmt, msgmerge)`), ohne gettext im PATH Abbruch mit Hinweis. Nachweis: nur Build-Tooling; gebaut, 5 Python-Tests grün, `--check` auf Windows 10 gelaufen; keine APK gebaut, keine neue Version; Headset-Abnahme nicht nötig (App unverändert).
- **Builder-Oberfläche (SETUP-001):** Der grafische Builder zeigt nach „Voraussetzungen prüfen“ (und beim Build) das gettext-Ergebnis als eigene, fett und farbig hervorgehobene Zeile unter dem Status – „gettext: OK“ oder z. B. „gettext fehlt: msgfmt – Installationshinweis: Windows: winget install --id mlocati.GetText …“ –, zusätzlich zum Protokoll und ohne Pfade. Die Auswertung der `--check`-Zeile steckt als reine Funktion `gettext_status()` in `scripts/builder_support.py`. Nachbesserung: Unter Windows schrieb `build-quest.py` die Fehlermeldung in der ANSI-Codepage (cp1252), der Builder las UTF-8 – aus „–“ wurde U+FFFD, die Statuszeile fiel auf „nicht geprüft“ zurück und das Protokoll war unlesbar. Der Builder startet das Skript jetzt mit `PYTHONIOENCODING=utf-8` (`child_process_env()`), und die Auswertung akzeptiert beliebige Trennzeichen nach „im PATH“. Nachweis: gebaut; 9 Python-Tests grün (4 neu: OK-Zeile, fehlendes Werkzeug je Plattform ohne Pfade, Fehlerfall über die Prozessgrenze, verstümmeltes Trennzeichen), `py_compile` des Builders fehlerfrei, `build-quest.py --check` auf Windows 10 unverändert durchgelaufen (exit 0); Fehlerfall über die Prozessgrenze geprüft (echtes `build-quest.py --check` ohne gettext im PATH: exit 1, Ausgabe ohne U+FFFD, Statuszeile „gettext fehlt: msgfmt, msgmerge – Installationshinweis: …“); Oberfläche nicht interaktiv gestartet; keine APK gebaut, App unverändert.
- **Nachprüfung Builder (SETUP-001):** Der gettext-Fehlerfall wurde über denselben Aufrufweg wie der grafische Builder nachgeprüft (Kindprozess `build-quest.py --check` mit `child_process_env()`, UTF-8-Dekodierung wie im Builder, Auswertung zeilenweise mit `gettext_status()`). Ohne gettext im PATH: exit 1, kein U+FFFD, Statuszeile „gettext fehlt: msgfmt, msgmerge – Installationshinweis: Windows: winget install --id mlocati.GetText (danach Builder neu öffnen)“; mit gettext: exit 0, „gettext: OK“. Kein Code geändert. Nachweis: gebaut; 9 Python-Tests grün, `py_compile` fehlerfrei, Windows 10; Oberfläche nicht interaktiv gestartet; keine APK gebaut, App unverändert, Headset-Abnahme nicht nötig.
- **Dokumentation (DOC-003):** README und Wiki werben nicht mehr mit einem Linkshändermodus: In der README-Komfortzeile ist er gestrichen, in `docs/wiki/VR-Verbesserungen.md` heißt der Bereich jetzt „Tastenbelegung (inklusive Wahl der Zeigerhand)“ mit dem Hinweis, dass Kartenziehen und Rahmenbewegung mit linkem Zeiger noch nicht umgesetzt sind (VR-006); derselbe Hinweis steht in `docs/wiki/VR-Komfort.md` bei „Zeigerhand“. Nachweis: Nur Dokumentation; keine APK gebaut, App unverändert, Headset-Abnahme nicht nötig.
- **Dokumentation (DOC-001):** Neue [STATUS.md](STATUS.md) mit Rückroll-Punkt `v2.1.0-preview.8` und getrennter Übersicht gebaut / installiert / im Headset bestätigt für preview.8 bis .13; Tabelle „Als Nächstes“ in BACKLOG.md wieder durchgehend; README verweist auf STATUS.md. Nachweis: nur Dokumentation, keine Tests nötig; keine APK gebaut.
- **Dokumentation (DOC-002):** README nennt jetzt Version 2.1.0-preview.13 (stand noch auf preview.4); Sammel-Nachweis dieses Abschnitts durch je einen eigenen Nachweis pro Eintrag ersetzt; in STATUS.md ist „installiert“ für preview.10 jetzt „unklar“ (CHANGELOG sagt „noch nicht auf die Quest übertragen“, die Start-Schleife aus preview.11 wird aber preview.10 zugeordnet). Nachweis: nur Dokumentation; vorhandene Python-Tests weiter grün; keine APK gebaut.

- Konverter `scripts/convert-hq-glb.py` für das 3D-Hauptquartier wiederhergestellt (war beim Zusammenführen der Zweige verloren gegangen); die Quell-Dreieckszahl im Bericht wird jetzt gezählt statt fest eingetragen. Anleitung in `docs/wiki/Diorama-und-Ansichten.md`.

## 2.1.0-preview.13 — 2026-09-28

Code-Prüfung aller Änderungen seit `v2.1.0-preview.8` mit drei Korrekturen:
- **Neustart nach Absturz:** Android erzeugt die Spiel-Activity nach einem Absturz mit ihrem ursprünglichen Intent neu; die Markierung „regulärer Start“ aus preview.11 verhinderte dadurch genau die gewünschte Umleitung. Jetzt entscheidet nur die (verlässlich gelesene) Pfadvariable, und pro Prozess wird höchstens einmal umgeleitet – eine Schleife ist ausgeschlossen.
- **Echter Tisch ausschalten:** Das Brett wird danach wieder vor dir zentriert, statt geneigt an der Tischposition zu bleiben.
- **Aufnahme:** Ein neuer Start, während die vorige Datei noch abgeschlossen wird, wird abgelehnt statt die Encoder zu teilen (sonst Absturzgefahr). Beim Start des Schreibens wird ein Schlüsselbild angefordert, damit der Videoanfang nicht fehlt.

Smoke-Test auf der Quest (Brille im Ruhezustand): Installation und regulärer Start ohne Absturz, ohne ANR, ohne Start-Schleife; Prozess nach über einer Minute stabil (je Start zwei Einträge, weil die Quest-Oberfläche Starts per PendingIntent wiederholt). Der simulierte Absturz-Neustart (Spiel-Activity direkt) wurde von der Quest mit „Controller erforderlich“ abgefangen und ist noch mit aufgesetzter Brille zu wiederholen. Echter Tisch, Tisch-Simulator und Video-Aufnahme sind weiterhin nicht im Headset abgenommen.

## 2.1.0-preview.12 — 2026-09-28

- **Video-Aufnahme im Spiel:** Linke Menütaste → Favoritenseite → **„Video aufnehmen (Spielbild + Ton)“**, erneut drücken beendet. Aufgenommen wird das flache Spielbild (max. 1920 px breit, 30 fps, H.264) zusammen mit dem gemischten Spielton (AAC, direkt aus SDL_mixer, kein Mikrofon) als MP4 unter `/sdcard/RTTR/videos/xr.rttr-JJJJMMTT-HHMMSS.mp4`. Ohne Zimmer, Hände, Controllerstrahl und Systemfenster.
- Während der Aufnahme leuchtet in der Brille ein roter Punkt oben links am Spielfenster; er ist nicht Teil des Videos. Beim Beenden des Spiels wird eine laufende Aufnahme sauber abgeschlossen.
- Technik: Android-Hardware-Encoder (`MediaCodec`/`MediaMuxer`) in `VideoRecorder.java`; der OpenXR-Treiber zeichnet das fertige Panelbild über einen eigenen, geteilten EGL-Kontext mit nativen GLES-Aufrufen in die Encoder-Oberfläche, sodass der gl4es-Zustand des Spiels unberührt bleibt. Der SDL-Audiotreiber reicht den Mix per `Mix_SetPostMix` weiter. Treiber-ABI 24.
- Musik vor der Aufnahme ggf. ausschalten (Content ID bei YouTube).

Gebaut, noch nicht auf die Quest übertragen und nicht im Headset getestet.

## 2.1.0-preview.11 — 2026-09-28

- **Mögliche Endlosschleife beim Start behoben (Regression aus preview.10):** Die neue Absturz-Absicherung in `SDLActivity` las die Spielpfade mit `System.getenv`, der Startbildschirm setzt sie aber per `Os.setenv`. Liefert Java dabei einen alten Stand, wechselten Startbildschirm und Spiel endlos hin und her (Nutzer: App in Schleife, Meta-Taste ohne Wirkung). Jetzt `Os.getenv` und zusätzlich eine Intent-Markierung für den regulären Start – ein regulärer Start leitet nie um.

Gebaut, noch nicht auf die Quest übertragen. Ursache noch per Headset-Protokoll zu bestätigen.

## 2.1.0-preview.10 — 2026-09-27

- **Absturz behoben:** Das Öffnen des Tisch-Simulators (seit preview.7) brach das Spiel ab, weil im Fenster Steuerelement-Kennungen doppelt vergeben waren (Überschrift/Zeilenbeschriftungen vs. Voreinstellungs-Knöpfe). Ursache per Absturzprotokoll und Debug-Symbolen ermittelt.
- **Folgeabsturz abgesichert:** Startet Android das Spiel nach einem Absturz direkt neu, fehlen die Spielpfade und die Engine brach ab (System::getHomePath). Das Spiel leitet jetzt auf den regulären Start um.

- **Anzeigename jetzt `xr.rttr`** (= App-ID) nach der Namenskonvention für alle XR-Ports (Nutzerentscheidung 27.09.2026; XR-OpenRA heißt `xr.openra`). Gilt für Launcher, „Unbekannte Quellen“ und alle Activities. App-ID unverändert, Spielstände und Einstellungen bleiben.

Gebaut, noch **nicht** auf die Quest übertragen (Nutzer testet gerade).

## 2.1.0-preview.9 — 2026-09-27

- **Echter Tisch (VR-022, Stufen 1+2):** Unter Fenster → Darstellung/Tisch → **Tisch-Quelle: Echter Tisch (Raumscan)**. Die App liest den Raumscan der Quest (Szenenmodell), sucht Objekte mit der Bezeichnung Tisch/Schreibtisch in bis zu 3 m Entfernung und legt das Spielbrett flach auf die größte Tischfläche: mittig, 1 cm über der Oberfläche, eine Tischkante parallel zur Bildkante, obere Kartenseite von dir weg, Breite passend zur Fläche (max. 2,4 m). Strahl und Klicks nutzen dieselbe Lage.
- Die Berechtigung „Raumdaten“ (`com.oculus.permission.USE_SCENE`) wird erst beim Einschalten der Option einmalig im Headset angefragt. Ohne Scan oder ohne Tisch bleibt der simulierte Tisch aktiv; alle 10 s wird erneut gesucht. „Zentrieren“ sucht den Tisch neu, eine Neuzentrierung durch die Quest ebenso.
- Statuszeile im Menü: nicht unterstützt / Freigabe erlauben / suche / keiner gefunden / gefunden.
- Neue OpenXR-Erweiterungen, nur wenn vorhanden: `XR_FB_spatial_entity`, `XR_FB_spatial_entity_query`, `XR_FB_spatial_entity_storage`, `XR_FB_scene`. Treiber-ABI 23.
- Test `tests/test_vr_realtable.cpp` (Ausrichtung, Einpassen, Obergrenze, Pose) und `test_vr_navigation.cpp` auf der Quest selbst als ARM64-Programm ausgeführt: bestanden.

APK am 27.09.2026 um 23:32 installiert, 121 Benutzerdateien vorher gesichert. Tischerkennung und Lage im Headset noch nicht abgenommen.

## 2.1.0-preview.8 — 2026-09-27

- **Screenshot-Modus** im VR-Schnellmenü (Favoritenseite, neben Passthrough): Solange kein Fenster offen ist, sind Controllerstrahl, Zielring und Mauszeiger ausgeblendet; die Eingabe bleibt aktiv. Ein kurzer Fokusverlust durch die Quest-Screenshot-Kombination (Meta-Taste + Trigger) pausiert das Spiel nicht mehr, daher keine Meldung „Spiel wurde pausiert“ und keine Weiterspielen-Frage. Beim Öffnen des Schnellmenüs erscheint der Strahl wieder. Nicht gespeichert: nach Neustart aus. Treiber-ABI 22.
- Bekannte Grenze: Das kleine Systemfenster der Quest („App-Name nicht verfügbar / Beenden“) gehört zur Meta-Oberfläche und kann von der App nicht unterdrückt werden.

APK am 27.09.2026 um 22:58 installiert, 121 Benutzerdateien vorher gesichert. Vom Nutzer als funktionierender Stand freigegeben und als GitHub-Vorabversion `v2.1.0-preview.8` veröffentlicht (privates Repository). Einzelabnahme von Screenshot-Modus, Tisch-Simulator, Scrolltempo und 4×-Zoom steht noch aus.

## 2.1.0-preview.7 — 2026-09-27

- **Tisch-Simulator** im VR-Schnellmenü (linke Menütaste → „Tisch-Simulator“): vier Voreinstellungen – Schreibtisch (45°, 40 cm, 0,8 m), Couchtisch (30°, 70 cm, 1,2 m), Stehpult (60°, 20 cm, 0,8 m), Großer Tisch (30°, 50 cm, 1,8 m) – sowie −/+ für Neigung, Tischmitte unter Blickhöhe und Abstand. Jede Änderung wirkt sofort, schaltet bei Bedarf auf die Tischansicht, wird gespeichert und bleibt im offenen Fenster sichtbar; ein neuer Abstand zentriert das Brett erneut vor dir.

APK am 27.09.2026 um 22:48 installiert, 121 Benutzerdateien vorher gesichert. Noch nicht im Headset abgenommen.

## 2.1.0-preview.6 — 2026-09-27

- Neue Option unter Controller → Navigation: **Rechter Griff: Scrolltempo** 1× (Standard, Karte folgt dem Zeiger 1:1), 2×, 3×, 4×. Skaliert wird nur die Zeigerbewegung des jeweiligen Frames, dadurch kein Aufschaukeln. Treiber-ABI 21.
- Zusätzliche Zoomstufe **4×** (bisher max. 3×) für Nahansicht; wirkt auf X, linken Griff + Stick und Trigger-Greifen.
- README-Galerie mit sechs Quest-Aufnahmen (Leinwand, Tisch, Schnellmenü, Übersicht, VR-Komfort, Bildoptionen).
- Befund: Die Quest-Systemleiste zeigt im laufenden Spiel weiterhin „App-Name nicht verfügbar“, obwohl „Unbekannte Quellen“ den Namen „xr.rttr“ korrekt anzeigt. Vermutlich eine Eigenheit der Systemoberfläche bei seitlich installierten Apps; Vergleich mit anderer seitlich installierter VR-App steht aus.

APK am 27.09.2026 um 22:43 installiert, 121 Benutzerdateien vorher gesichert. Neue Optionen noch nicht im Headset abgenommen.

## 2.1.0-preview.5 — 2026-09-27

- App-Name jetzt „xr.rttr“ und bei allen Activities gesetzt. Vorher hatten Start- und Spiel-Activity ein leeres Label, deshalb zeigte die Quest-Systemleiste (unten rechts, mit dem Beenden-Knopf) keinen Namen.

APK am 27.09.2026 um 21:59 auf die Quest installiert. Die App war zuvor deinstalliert; Spieldaten und Spielstände unter `/sdcard/RTTR` waren unverändert vorhanden. Pfade und „Alle Dateien“ neu gesetzt. Anzeige des Namens in der Systemleiste noch nicht im Headset bestätigt.

## 2.1.0-preview.4 — 2026-09-27

- Paket-ID jetzt `xr.rttr` (vorher `org.s25rttr.sdl`); der Java-Namespace bleibt. Regulärer Start: `xr.rttr/org.s25rttr.sdl.GameStartActivity`. Der Quest-Screenshot-Ordner heißt entsprechend.
- Jeder App-Start beginnt als Leinwand, zentriert vor der aktuellen Blickrichtung, sobald gültiges Kopftracking vorliegt (nicht erst mit Eingabefokus). Zuletzt gewählte Tischansicht sowie gespeicherte Position/Ausrichtung werden nicht übernommen; Breite und Abstand bleiben gemerkt. Die Option „Start vor Blickrichtung“ entfällt, „Fensterposition merken“ heißt jetzt „Fenstergröße merken“.
- Neue Option unter Controller → Navigation: **Trigger halten: Karte greifen** (Standard aus). Trigger der Zeigerhand auf der freien Karte halten und Strahl bewegen verschiebt die Karte; der gegriffene Kartenpunkt bleibt unter dem Strahl. Hand 6 cm zum Fenster hin zoomt eine Stufe hinein, 6 cm zurück eine Stufe heraus (kurzer Vibrationsimpuls je Stufe). Kurzes Tippen ohne Bewegung bleibt ein normaler Linksklick, ausgelöst beim Loslassen. Über Fenstern, Bedienleiste und modalen Dialogen unverändert. Treiber-ABI 20.
- Erster Windows-Build: bzip2 nur als Bibliothek (keine Symlinks), Platzhalterpfad der Engine ohne Laufwerksbuchstaben, gettext (`msgfmt`) als Build-Voraussetzung.

Build unter Windows 10 erfolgreich; Manifest bestätigt `versionCode=6`. Python-Tests bestanden; nativer Navigationstest nur per NDK-Syntaxprüfung (unter Windows nicht ausgeführt). APK am 27.09.2026 um 20:17 auf Quest installiert, 100 Benutzerdateien vorher gesichert. Nutzer bestätigt im Headset: Menüs wieder korrekt. Zentrierter Start und Trigger-Greifen noch nicht einzeln abgenommen.

## 2.1.0-preview.3 — 2026-09-26

- Gemeinsames Raster für Bild- und Komfortmenüs: 40 Pixel Zeilenabstand, 32 Pixel Feldhöhe, gleiche Eingabespalte und erster Eintrag bei Y=115.
- Controllergruppe mit Tasten, Achsen und Navigation. Pro Taste stehen Kurz- und Langdruck direkt untereinander.
- Fenstergruppe mit Darstellung/Tisch sowie Position/Abstand. Merken, Zentrieren und Rahmenbewegung befinden sich gemeinsam unter Position.
- Bildmenü in Bild/Fenster und Strahl/Zeiger unterteilt, ohne dicht gedrängte zehnzeilige Liste.
- Tischmitte standardmäßig 40 statt 65 cm unter Blickhöhe, also 25 cm höher (rund 38 % weniger Absenkung). Einstellbar von 0 bis 90 cm in 10-cm-Schritten unter Fenster → Darstellung/Tisch.
- Alte Höhenprofile werden einmalig höher zugeordnet: 40→20, 65→40, 90→60 cm. Eine danach gewählte Zentimeterhöhe bleibt erhalten. Tischhöhe bezeichnet die Mitte relativ zur zentrierten Blickhöhe, nicht die obere Bildschirmkante.
- Schnellmenü ebenfalls mit einheitlichen 40-Pixel-Zeilen und 32-Pixel-Schaltflächen.
- Online-Gebäudehilfe unter Komfort: Aus (Standard) oder eine von vier Websites. In Gebäudeinformationen erscheint dann ein Browser-Button; gilt auch für Lager, Militärgebäude und Baustellen. Die ausgewählte Übersichtsseite wird geöffnet, kein ungeprüfter Gebäude-Anker.
- Beta-Schalter unter Fenster → Darstellung/Tisch: externes HQ-Modell an der Kartenposition, mit Kartenzoom und Navigation. Standard aus, nur Tischansicht und geschlossene Spielmenüs; bei fehlendem Modell oder abgeschnittenem HQ bleibt der Originalsprite. Separate Menü-Layer und alte Griffsteuerung wurden nicht übernommen.
- HQ-Modell nutzt einen separaten Tiefenpuffer pro Auge; private Assetdatei bleibt außerhalb der APK und des Repositorys.
- VR-Strahltest auf alle zehn Höhen erweitert: 38.880 Treffer-/Rückseitenfälle bestanden. Headset-Abnahme der neuen Menüs und Höhen steht aus.

Build erfolgreich; Manifest bestätigt `versionCode=5`. Drei Python- und sieben native Tests bestanden. APK am 26.09.2026 um 22:31 auf Quest installiert; 89 Benutzerdateien gesichert und unverändert bestätigt. Regulärer Start ausgelöst, Prozess vorhanden; Activity anschließend ohne Fokus gestoppt, Rendering noch nicht bestätigt.

## 2.1.0-preview.2 — 2026-09-26

- Mauszeiger, Controllerstrahl und Zielmarkierung während Splash/Intro, Kartenladen und ohne interaktiven Desktop ausblenden. Gehaltene Eingaben müssen nach dem Laden zunächst losgelassen werden.
- Bildmenü und Komfortseiten: gemeinsame Größe 640 × 560, erste Einstellungszeile bei Y=95 und Eingabespalte X=300; doppelten Achsenschalter im Bildmenü entfernt.
- System-Neuzentrierung via OpenXR-Referenzraum-Ereignis: ab dessen `changeTime` Fensterbreite 2 m, gewählter Startabstand und Position vor dem Kopf wiederherstellen. Manueller Zentrierbefehl setzt ebenfalls vollständig zurück. Normale Starts berücksichtigen weiterhin gespeicherte Werte.
- Passthrough als separater Hintergrund wieder aufgenommen; Favoritenmenü mit Schalter und Regler 0–100 % in 10-%-Stufen. Regler verändert die Umgebung, nicht die Deckkraft des Spielfensters. Passthrough auch als frei belegbare Favoriten-/Tastenaktion verfügbar.
- Keine separaten Menü-Layer wieder eingeführt.

Build erfolgreich; APK-Manifest geprüft (`versionCode=4`). Sechs bestehende native Tests und drei Python-Tests bestanden. APK am 26.09.2026 um 22:07 installiert; Start im Ruhezustand noch nicht bestätigt. Menüausrichtung, Ladezeiger, Passthrough-Deckkraft und insbesondere das Meta-Neuzentrierungsereignis müssen auf der Quest praktisch abgenommen werden.

## 2.1.0-preview.1 — 2026-09-26

Erster eindeutig versionierter Prüfstand nach dem schrittweisen Rückbau. Vorabversion; keine vollständige Headset-Abnahme.

### Enthalten

- Ladebildschirm mit Ladeschritten, Fortschrittsbalken und Quest-Geduldshinweis.
- Gemeinsame Achseneinstellungen für beide Controller: X/Y-Tausch und getrennte Richtungsumkehr.
- Linker Grip + Stick vor/zurück zoomt die Karte unabhängig vom X/Y-Tausch; kein zyklischer Sprung an der Zoomgrenze. Die X-Taste behält ihren bisherigen Zoomzyklus.
- Rechter Grip hält den Rechtsklick wieder während der Kartenbewegung.
- Optionales Verschieben am Originalrahmen: reine Translation, keine Drehung oder Formänderung. Derzeit bei rechtem Zeiger und geschlossenen Spielmenüs.
- Fenstergröße/Abstand per Stick standardmäßig gesperrt, optional ohne Grip aktivierbar.
- Backlog, Updatelog und zentrale Android-Versionierung eingeführt.

### Bewusst zurückgestellt

Separate aufrechte Menü-Layer, Passthrough und 3D-Gebäude sind in dieser Arbeitsbasis nicht enthalten. Sie werden erst nach bestätigten Eingaben einzeln wieder aufgenommen.

### Prüfstand

APK 2.1.0-preview.1 gebaut; Manifest bestätigt versionCode 3. Diese APK wurde noch nicht installiert.

Sechs native Testprogramme und drei Python-Tests bestanden. Die funktionsgleiche Vorgänger-APK ohne neue Versionsmetadaten wurde am 26.09.2026 um 21:52 Uhr installiert und gestartet; Daten unverändert, Crash-Puffer leer. Die praktische Bestätigung von Zoom, Kartenziehen und Rahmenbewegung steht noch aus. Installation ist kein Nachweis der Bedienbarkeit.

## Historische Vorabstände — 2026-09-26

Diese APKs trugen alle `versionName=2.0.2`, `versionCode=2`. Die folgenden Bezeichnungen sind Build-Namen und keine nachträglich erfundenen Versionsnummern.

| Stand | Änderung / Ergebnis |
| --- | --- |
| `navigation` / `68123ef` | Controller-Navigation, Startpositionen und automatische Speichernamen; gewählte Basis für den Rückbau. |
| `room`, `grab` | Passthrough, separate Menüs und rotierbarer Fenstergriff; später wegen Menü- und Bedienproblemen zurückgestellt. |
| `hq3d`, `hq3d-detail`, `hyper3d-hq` | Experimente mit 3D-Hauptquartier; Detail-Build nur vorbereitet, andere Varianten auf Quest installiert. |
| Rückbau um 21:24 | Rückkehr auf `hq3d` reichte nicht aus; Menüfehler waren bereits enthalten. |
| Rückbau um 21:30 | `navigation` mit damaligen VR-Einstellungen installiert; aktuelle Spielstände erhalten. |
| `baseline-loading-axes-zoom`, installiert 21:40 | Ladeanzeige und Achsen wieder aufgenommen; verzögerter Rechtsklick und Zoomachsen führten zu gemeldeten Eingaberegressionen. |
| `grip-fix-frame-move`, installiert 21:52 | Rechtsklick-Halten wiederhergestellt, Zoom auf physisch vor/zurück, reine Rahmenverschiebung ergänzt. Headset-Rückmeldung ausstehend. |

Weitere frühere Schritte und Testdetails stehen in [VALIDATION.md](docs/VALIDATION.md) und der Git-Historie. Historische Testeinträge gelten nur für ihren jeweiligen Build.

## Versionsregeln

- `versionName`: `MAJOR.MINOR.PATCH`, in der Testphase mit `-preview.N`.
- `versionCode`: positive, bei jeder neu verteilten APK streng steigende Ganzzahl; unabhängig vom Namen.
- Beide Werte ausschließlich in `version.properties` pflegen; Gradle übernimmt sie in das APK-Manifest.
- Die Treiber-ABI `DRIVERAPIVERSION` ist eine getrennte technische Kompatibilitätsnummer.
- Jede neue APK mit Versionsname und SHA-256 archivieren. Änderungen, bekannte Grenzen und tatsächlichen Prüfstatus vor Veröffentlichung hier ergänzen.
- Eine Versionsänderung im Repository ändert eine bereits installierte APK nicht automatisch.
