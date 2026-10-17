# Prüfung des Repository-Aufbaus

Stand: 26. September 2026, ergänzt am 27. September 2026.

## preview.4 – erster Windows-Build (27.09.2026)
- Host: Windows 10 Pro 19045, Temurin JDK 17.0.20, Python 3.13.15, Android SDK Platform 36, Build-Tools 35.0.0, NDK 27.0.12077973, CMake 3.22.1, gettext 1.0 (mlocati). Frischer rekursiver Klon inklusive aller Submodule.
- `build-quest.py --check` bestanden, danach vollständiger CLI-Build (Abhängigkeiten, OpenXR-Loader, Engine) erfolgreich. Behobene Windows-Fehler: bzip2-Symlinks, Platzhalterpfad `<build>/D:/…`, fehlendes Python im PATH, fehlendes gettext. GUI-Builder unter Windows nicht ausgeführt.
- Manifest: `xr.rttr`, `versionCode=6`, `versionName=2.1.0-preview.4`. Drei Python-Tests bestanden; `test_vr_navigation.cpp` (inkl. neuer MapGrab-Fälle) nur per NDK-Clang `-fsyntax-only -Wall -Wextra` geprüft, unter Windows nicht ausgeführt.
- Quest 3: vor Installation 100 Benutzerdateien gesichert, APK um 20:17 installiert und regulär gestartet; Pfade und „Alle Dateien“ beibehalten. Nutzer bestätigt: Menüs wieder in normaler Größe. Zentrierter Start und Trigger-Greifen noch nicht einzeln abgenommen.
- Lehre: Ein zuvor versehentlich installierter Build von `main` (2.0.2) zeigte riesige separate Menüebenen, sichtbaren Strahl beim Laden und unerreichbare VR-Menüs – vor jedem Test installierte `versionName` prüfen.

- Vollständiger ARM64-Debug-Build aus einem separaten Checkout ohne bestehende native Build-Ausgaben erfolgreich; Gradle: 40 Tasks ausgeführt, 8 min 57 s. OpenXR-Loader wurde zusätzlich aus dem festgelegten SDK-Quellstand gebaut.
- Host: macOS, JDK 17.0.14, Android SDK 36, NDK 27.0.12077973, CMake 3.22.1. Toolchain- und Gradle-Download-Caches waren vorhanden; die Submodule wurden aus lokal vorhandenen Git-Objekten auf die festgelegten Revisionen ausgecheckt. Ein kompletter Netzwerk-Neudownload aller Quellen wurde nicht geprüft.
- Alle 88 direkten Submodule stimmen mit SOURCE-PROVENANCE.json überein; verschachtelte Abhängigkeiten wurden rekursiv initialisiert.
- Eigene Upload-Dateien gegen die lokal vorhandenen Originalspieldaten per SHA-256 abgeglichen: keine identischen Dateien gefunden. Das ist eine ergänzende Kontrolle, kein allgemeiner Rechtsnachweis; Lizenzangaben der Upstream-Dateien bleiben maßgeblich.
- Suche nach gängigen Token-/Privatschlüsselmustern ohne Treffer. Originaldaten, private Spielstände, Logs, APKs und vorgebaute native Bibliotheken nicht im Quellcode-Commit enthalten. Die APK wird separat als private Vorabversion angeboten.
- Interne Dokumentationslinks und Python-Syntax geprüft. ADB-Skript im Trockenlauf geprüft: bestehende Daten standardmäßig abgelehnt; explizite Übertragung auf ausgewähltes Gerät; keine Deinstallation oder Löschung.
- Upstream-Quellen enthalten bestehende Compiler-/CMake-/Whitespace-Warnungen. Neue Dokumentation und Hilfsskripte bestehen den Whitespace-Check. Kein vollständiger automatisierter Engine-Testlauf durchgeführt.
- Die bisherige Entwicklungsfassung lief auf der Quest; dieser neue vollständige Build wurde noch nicht erneut im Headset abgenommen. Windows-Build und vollständige Neueinrichtung mit frischem Meta-Konto sind nicht getestet.

## Grafischer APK-Builder
- Python-/Tkinter-GUI auf macOS gestartet; gemeinsame Preflight-Prüfung mit echter Toolchain erfolgreich. Drei Offline-Tests für Pfadauswahl (einschließlich Leerzeichen/macOS-JDK-Paket), fehlende Werkzeuge und falsche Java-Version bestanden.
- Tk-Laufzeittest: Hauptbedienelemente innerhalb der Standardfensterbreite, fehlgeschlagener Build bei vorhandener alter APK korrekt als Fehler, Testprozess nach Abbruch beendet. Keine Screenshot-Abnahme vorgenommen.
- Echter APK-Build über GUI-Worker und CLI mit expliziten SDK-/Java-Pfaden erfolgreich. Dafür vorhandene native Caches des vollständigen Prüf-Checkouts verwendet.
- SH-Launcher und alle 13 macOS-Befehlsblöcke per `sh -n` geprüft; PowerShell-Launcher und alle 14 PowerShell-Befehlsblöcke mit dem PowerShell-Parser syntaktisch geprüft. Python-Module kompilieren. Windows-PowerShell/CMD und Linux nicht praktisch ausgeführt. Die neuen vollständigen Paketmanager-Anleitungen sind noch nicht auf frisch eingerichteten Rechnern getestet.

## VR-Optionen und Fensterwölbung
- Native ARM64-APK mit Controller-Achsentausch, Bildkorrektur, optionalem Cylinder-Layer und Zeigeroptionen erfolgreich gebaut. Alle Treiber verwenden die neue Callback-ABI 9.
- Geometrietest: 90 Treffer (flach, 45°, 75°; Mitte/Ränder und versetzter Controller) sowie 12 Ablehnungsfälle bestanden. Aufruf: `c++ -std=c++17 -Wall -Wextra -Werror -I android/app/jni/s25client/extras/videoDrivers/OpenXR tests/test_vr_panel.cpp -o /tmp/test_vr_panel && /tmp/test_vr_panel`.
- VR-Vorschau liegt getrennt von den gespeicherten Einstellungen; nur Übernehmen kopiert sie in die dauerhafte Konfiguration.
- Die subjektive Darstellung, Auswahl aller Zeigerformen und Randklicks auf der gewölbten Fläche müssen im Headset abgenommen werden.
- APK-Dateiname und App-Label mit `aapt dump badging` geprüft: xr.rttr-Quest.apk / xr.rttr, Paketkennung unverändert. Auf der verbundenen Quest per Update installiert und regulär gestartet; zuvor 49 Benutzerdaten-Dateien gesichert.
- Quest-Screenshot zeigt das integrierte VR-Fenster. Ausgewählte VR-Werte wurden nach Übernehmen in CONFIG.INI gespeichert (u. a. Achsentausch, 80 % Helligkeit, 120 % Kontrast und leichte Wölbung). Keine vollständige Abnahme sämtlicher Kombinationen.

## Screenshot-Standardprofil, Maus-Icon aus und Ultrawide
- Neuester vom Nutzer gespeicherter Quest-Screenshot per ADB geladen und visuell geprüft. Die neun sichtbaren VR-Werte sind jetzt die gemeinsamen Defaults für Neuinstallation, fehlende Einträge und Zurücksetzen; vorhandene gespeicherte Werte werden beibehalten.
- Maus-Icon-Auswahl um `Aus` erweitert (persistenter Wert 4, bestehende Größenindizes unverändert). Die Zeichenfunktion überspringt dann nur das Spielsymbol.
- Native APK mit 2560 × 1080, 3440 × 1440 und 3840 × 1080 erfolgreich gebaut, per Update installiert und gestartet. Spielframebuffer und Panel-Swapchain wechseln gemeinsam; fehlgeschlagene Neuanlage erhält die vorherigen Ressourcen. Startauflösung berücksichtigt unterstützte gespeicherte Werte.
- Geometrietest auf fünf Seitenverhältnisse erweitert: 450 Treffer und 60 Ablehnungsfälle bestanden. Der Test deckt flache, leicht und stärker gewölbte Flächen ab.
- Vor Installation 54 Benutzerdaten-Dateien gesichert. App-Prozess läuft nach Update; gespeichertes VR-Profil unverändert vorhanden. Vollständige Headset-Abnahme aller neuen Auflösungen und der Aus-Option steht noch aus.

## VR-Eingabe und Bildschärfe – 26. September 2026
- Android/arm64-Debug-APK erfolgreich gebaut; Preflight mit JDK 17/SDK 36/NDK 27 bestanden.
- Drei Python-Buildertests bestanden; `git diff --check` ohne Fehler.
- Quest 3 per ADB verbunden. 56 Benutzerdateien vor dem Update gesichert, Installation mit `adb install -r` erfolgreich.
- Regulärer Start angefordert; bislang kein laufender App-Prozess bestätigt. Noch keine praktische Abnahme der neuen Tastatur, X/A/B-Belegung, Filterstufen, Cursorfransen oder höchsten Auflösungen.
- Neue Funktionen: gemeinsame ctrlEdit-Tastatur, linkes VR-Schnellmenü, X-Kartenzoom, A-Kontextmenü, B-Spielmenü, höhere GUI-Skalierung über zusätzliche Renderauflösungen, einstellbare Bildschärfe mit Vorschau und Persistenz.
- Der Originalrand des Finger-Sprites bleibt; die Änderung entfernt nur schwach deckende Fransen an der skalierten Transparenzkante.

## Wählbare Startdistanz – 26. September 2026
- Nah 1,2 m / Mittel 1,8 m / Weit 2,8 m im vorhandenen VR-Menü; Mittel bleibt Vorgabe. Auswahl wird als `vr_start_distance` gespeichert und beim ersten Frame sowie Fenster-Reset verwendet.
- Android-APK erfolgreich gebaut, Preflight und drei Buildertests bestanden; Diff ohne Formatierungsfehler.
- Headset-Abnahme von Startdistanz und Stick-Reset steht aus.

## VR-Komfortpaket – 26. September 2026
- Implementiert: einzeln schaltbare Zeigerglättung, Klickstabilisierung, Haptik, zusätzliche Hover-Hervorhebung, Zeigerhand, Listenscrollen, Einzelspieler-Pause mit Fortsetzungsbestätigung, Fensterzentrierung/Positionsspeicherung, X/Y/A/B kurz/lang, sechs Favoriten und drei Grafikprofile. Linke Menütaste bleibt festes Schnellmenü.
- Sichere Auflösung: 15-s-Countdown, manuelles Zurück, Wiederherstellung von Auflösung/GUI und Speicherung des vorherigen Modus bis zur Bestätigung.
- Finaler Android/arm64-Build erfolgreich (25 s); Preflight mit JDK 17 / SDK 36 / NDK 27 bestanden.
- `test_vr_comfort.cpp`: 378 gedrehte flache/gewölbte Treffer, Kurz-/Langdruck ohne Doppelaktion, deaktivierte Aktionen, Verlust aktiver Eingabe, zeitabhängige Glättung und Countdown einschließlich Zeitgeberüberlauf bestanden.
- Bestehender Geometrietest: 450 Treffer / 60 Fehltreffer bestanden. Drei Python-Buildertests bestanden. Canonical `git diff --check` ohne Fehler.
- 56 Benutzerdateien vor dem Update gesichert. APK per `adb install -r` erfolgreich auf Quest 3 aktualisiert, regulärer Start angefordert. Zum Prüfzeitpunkt kein laufender App-Prozess und kein Eintrag im Crash-Puffer bestätigt.
- Praktische Abnahme von Menü, Haptik, Fokuswechsel, Grafikprofil-Rückfall und tatsächlicher Lesbarkeit bleibt ausstehend. Keine Windows-/Linux-/Headset-Abnahme daraus ableiten.

## 2026-09-26 – Leinwand und Tischansicht

- Nativer Android-Debug-Build auf macOS erfolgreich (OpenXR sowie Spiel-/SDL-Treiber gemeinsam, ABI 11).
- `python3 -m unittest discover -s tests -v`: 3 Builder-Tests bestanden. Build-Preflight mit konfiguriertem SDK/JDK bestanden.
- `test_vr_views.cpp`: 11.664 Treffer und ebenso viele Rückseiten-Ausschlüsse über beide Ansichten, vier Neigungen, drei Höhen, drei Abstände, drei horizontale Ausrichtungen, drei Breiten und zwei Seitenverhältnisse. Controller bleibt am Betrachter, während sich das Brett neigt. Zusätzlich feste Erwartungen für die horizontale Brettausrichtung, Erhalt der Bildmitte, Quaternion-/Strahlkonsistenz und unveränderte Leinwand-Pose.
- Bestehende Geometrietests weiterhin bestanden: 450 flache/gewölbte Treffer und 60 Fehlertreffer; 378 gedrehte Treffer plus Gesten-/Glättungs-/Countdown-Prüfung.
- Eigenständige C++-Tests mit `clang++ -std=c++17`, Include-Pfade `android/app/jni/s25client/extras/videoDrivers/OpenXR` und `android/app/jni/s25client/libs/driver/include` gebaut und ausgeführt.
- APK vorbereitet; für diesen Stand keine Installation und keine praktische Headset-Abnahme durchgeführt. Offen: Lesbarkeit geneigter Menüs, Randtreffer, Straßenbau-Ziehen, Tastatur und persistierte Einstellungen auf der Quest.
- Die Tischansicht stellt weiterhin das 2D-Spielbild dar. Echtes 3D-Gelände, separate Figuren und separate schwebende Menüs sind noch nicht implementiert.

## 2026-09-26 – Navigation, Startposition und Spielstandnamen

- Android-Build erfolgreich auf macOS, alle nativen Treiber mit ABI 12. Build-Preflight und 3 Builder-Tests bestanden.
- `test_vr_navigation.cpp`: 24 Kombinationen aus Bildrate (36/72/90/120), Tempo und Turbo; gleichmäßige Geschwindigkeit und begrenzte Diagonalgeschwindigkeit, Totzone, Doppeltipp-/Richtungswechsel-/Resetlogik, separate Ansichtsabstände sowie bereinigte Dateinamen inklusive UTF-8-Grenze bestanden.
- Bestehende 11.664 Ansichts-/Strahlfälle und 378 Komfort-/Gestenfälle weiterhin bestanden. `git diff --check` sauber.
- Vier aktuelle Gerätescreenshots und gespeicherte Einstellungen ausgewertet. Bilder und Benutzerkonfiguration bleiben lokal und wurden nicht veröffentlicht. Gespeicherte Bild-/Tischparameter werden beim Update erhalten.
- Benutzerdaten vor dem Update gesichert (Tar geprüft, 71 Einträge). APK per `adb install -r` installiert, regulärer Einstieg gestartet, laufender Prozess bestätigt. Crash-Puffer nach Start leer.
- Noch keine persönliche Headset-Abnahme der neuen Navigation oder des Speicherformulars: insbesondere Griff-Freigabe, invertierte Y-Achse, Listen gegen Kartenbewegung, Turbo und tatsächliches Speichern müssen im Spiel geprüft werden. Rechnerische Tests ersetzen dies nicht.

## Schrittweiser Wiederaufbau nach Rückkehr auf 68123ef

Arbeitsbranch: `fix/vr-baseline-20260926`. Wieder aufgenommen sind ausschließlich Ladeanzeige, beide Controllerachsen und Grip-Stick-Kartenzoom. Keine separaten Menü-Layer, kein Passthrough, kein Fenstergriff und keine 3D-Gebäude.

- VR-Komfort > Achsen: pro Controller X/Y-Tausch sowie Umkehrung der physischen X- und Y-Achse vor dem Tausch.
- Linker Grip + linker Stick: Kartenzoom über die zugeordnete Y-Achse, Wiederholung nach 450 ms, danach alle 250 ms. Bei aktivem X/Y-Tausch liegt dies auf der physischen X-Achse. Zoom endet an den vorhandenen Grenzen; X behält seinen bisherigen Zyklus.
- Rechter Grip + linker Stick: Fenstergröße/Abstand, sofern der Fenster-Schutz auf Griff steht. Rechter Grip ohne Verstellung löst Rechtsklick beim Loslassen aus.
- Ladeanzeige: tatsächliche Ladeschritte und Quest-Geduldshinweis.
- Achsen-/Zoom-Eingabetest sowie drei Builder-Tests bestanden. Build und praktische Quest-Abnahme separat prüfen.

APK-Build erfolgreich (1 min 27 s). Zusätzlich bestanden: 450 Panel-Treffer/60 Misses, 378 gedrehte Treffer, 24 Navigationsfälle sowie 11.664 Ansichts-/Strahlfälle. APK unter `.build/releases/baseline-loading-axes-zoom/`. Noch nicht auf der Quest installiert; praktische Abnahme ausstehend.

## Grip-Korrektur und Rahmenverschiebung

Screenshots 21:42–21:43 zeigen wieder vollständige Menüs und den originalen Zierrahmen. Eingaberegression: der verzögerte Rechtsklick verhinderte Kartenziehen, während der linke Achsentausch den Kartenzoom seitlich verschob.

- Links Grip + physisch vor/zurück: Kartenzoom, unabhängig vom X/Y-Tausch; Y-Umkehrung bleibt wirksam.
- Rechts Grip in der Karte: sofortiges RightDown und RightUp beim Loslassen, damit Cursorbewegung die Karte zieht.
- Rechts Grip am äußersten Originalrahmen: reine Translation entlang des Controllerstrahls mit festem anfänglichen Strahlabstand. Keine Änderung von yaw, pitch, Fensterbreite oder Wölbung. Griff in der Kartenmitte wechselt beim Überqueren des Randes nicht in Fensterbewegung.
- Rahmenbewegung nur bei geschlossenem Spielmenü und rechter Zeigerhand. Die untere mittlere Werkzeugleiste bleibt ausgenommen. Abschaltbar unter VR-Komfort > Sticks > Fenster am Rand verschieben.
- Stick-Verstellung der Fenstergröße/des Abstands ist standardmäßig gesperrt; optional ohne Grip aktivierbar. Beide Grips gehören jetzt dem Kartenzoom bzw. Kartenziehen/Rahmen.
- Neuer Test: 108 Translationsfälle plus Strahlschwenks und Rand-/Werkzeugleistenabgrenzung. Alter Standardwert-Test für die nun gesperrte Fensterverstellung angepasst.
- Headset-Abnahme der neuen Eingaben steht aus.

Grip-Korrektur: APK-Build erfolgreich, sechs native Testprogramme und drei Python-Tests bestanden. APK: `.build/releases/grip-fix-frame-move/xr.rttr-Quest.apk`. Noch nicht installiert, kein praktischer Headset-Test behauptet.

Nachtrag: Grip-Korrektur am 26.09.2026 um 21:52 auf Quest installiert und gestartet (damals 2.0.2/code 2). 80 Benutzerdateien gesichert und nach Installation unverändert geprüft; Prozess vorhanden, Crash-Puffer leer. Bedienabnahme weiter offen. Neue Versionsmetadaten 2.1.0-preview.1/code 3 betreffen den folgenden Build.

Version 2.1.0-preview.1: Android-Build erfolgreich (26 s), APK-Manifest mit aapt geprüft: versionName 2.1.0-preview.1, versionCode 3. APK separat versioniert archiviert. Nicht auf Quest installiert; dort bleibt der funktionsgleiche Build 2.0.2/code 2.

## 2.1.0-preview.2 – Ladezeiger, Menüpositionen, Neuzentrierung und Passthrough

- Neue Screenshots 21:54 geprüft; Bildmenü und Komfortmenü hatten unterschiedliche Breiten und obere Feldpositionen. Gemeinsame Rasterwerte implementiert. Neue Anordnung nicht im Headset abgenommen.
- Android-Build nach Anpassung der OpenXR-Strukturfelder erfolgreich (18 s beim abschließenden Build); Manifest mit aapt bestätigt: 2.1.0-preview.2 / Code 4.
- Sechs vorhandene native Geometrie-/Eingabetests und drei Python-Tests bestanden. Diese Tests simulieren weder Passthrough noch das System-Neuzentrierungsereignis.
- Ladezeiger werden sowohl beim Cursorzeichnen als auch beim Rendern des XR-Strahls geprüft; Eingaben während Lade-Desktops gesperrt.
- Passthrough verwendet `XR_FB_passthrough`, `XrPassthroughStyleFB.textureOpacityFactor` und einen eigenen Hintergrund-Layer, keine separaten GUI-Layer.
- Referenzraum-Neuzentrierung berücksichtigt `changeTime`; eine Zuordnung zur Meta-Taste ist eine Laufzeitannahme, die noch praktisch geprüft werden muss.
- Nicht installiert. Die Quest bleibt auf der zuletzt vom Nutzer beauftragten Grip-Korrektur.

Referenzen: [Meta: Passthrough](https://developers.meta.com/horizon/documentation/native/android/mobile-passthrough/), [Deckkraft](https://developers.meta.com/horizon/documentation/native/android/mobile-passthrough-customization/), [OpenXR-Spezifikation](https://registry.khronos.org/OpenXR/specs/1.1-khr/html/xrspec.html#XrEventDataReferenceSpaceChangePending).

Installation preview.2 am 26.09.2026 um 22:07: adb install -r erfolgreich; Paketmanager bestätigt 2.1.0-preview.2 / Code 4. 84 Benutzerdateien gesichert und nach Installation unverändert geprüft. Start angefordert, aber im Ruhezustand noch kein App-Prozess bestätigt; Crash-Puffer leer. Headset-Abnahme ausstehend.

## 2.1.0-preview.3 – Raster, Menügruppen und Tischhöhe

Nutzer bewertet preview.2 insgesamt positiv. Screenshots 22:15 zeigen funktionierendes Passthrough und vollständige Menüs, aber unterschiedliche Zeilenabstände (35/40/42/45/52/55/60 Pixel). Das neue gemeinsame Raster verwendet 40 Pixel. Dies bestätigt keine vollständige Einzelabnahme aller preview.2-Funktionen.

Neue Tischhöhe in Zentimetern, Migration der früheren drei Profile nach 20/40/60 cm. Tests auf zehn Höhen 0–90 cm erweitert; 38.880 Strahltreffer/Rückseitenfälle bestanden. Neue Menüstruktur statisch geprüft, praktische Darstellung und Höhenbedienung auf Quest noch offen.

### preview.3 – fertiger Build mit Gebäudehilfe und HQ-Beta

- APK gebaut; Manifest: `2.1.0-preview.3`, `versionCode=5`. Am 26.09.2026 um 22:31 auf der Quest installiert.
- Drei Python-Preflight-Tests und sieben native Testprogramme bestanden (Frame-Move, Restore, Panel, Comfort, Navigation, Views, HQ-Asset). Views: 38.880 Fälle; HQ: 49.999 Dreiecke und Verankerung bei Kartenzoom geprüft. Kein Ersatz für einen Render-/Bedientest im Headset.
- Read-only via ADB bestätigt: externes HQ-Asset unter `/sdcard/RTTR/vr-assets/headquarters.xrhq` vorhanden (18.120.208 Bytes). Keine Benutzerdaten verändert.
- Optionaler Web-Button an vier Gebäudefenster-Familien, feste HTTPS-Ziele, Android-Browseraufruf via SDL. Die vier Webseiten wurden geladen; Browserwechsel im Headset ist noch nicht getestet.
- Praktisch offen: Menüabstände und Schrift in allen GUI-Skalierungen; Höhe und Persistenz; Websitewechsel/Rückkehr; HQ-Tiefendarstellung, Position beim Verschieben und Zoom, Menüeinblendung und Bildrate.
- APK SHA-256: `b36180d309df353d10e37ed97a0d41d0e97829e9b7c0f2ae754abe37d74417f0`.

### preview.3 – Installation 22:31

- `adb install -r` erfolgreich, installiertes Manifest bestätigt Name `2.1.0-preview.3` und Code 5.
- Backup `.build/backups/before-preview3-20260926-223137/`: 89 Benutzerdateien; alle SHA-256-Hashes vor/nach Installation identisch.
- Regulärer Start über `.GameStartActivity`; Prozess 32022 vorhanden. SDL-Activity verliert unmittelbar Fokus und geht auf onStop; noch kein bestätigter Renderlauf. Sichtprüfung nach Aufsetzen der Quest ausstehend.
- Kein neuer Eintrag im Crash-Puffer seit Installationsbeginn. Historischer Eintrag von 22:11 ist kein Fehler dieses Starts.
