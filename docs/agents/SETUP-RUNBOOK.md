# Runbook: Konfiguration und Installation begleiten

## Auftrag und Zugriff klären
Den Nutzer nach Betriebssystem, gewünschtem Ziel (fertige APK installieren oder selbst bauen), vorhandenem Checkout und aktueller Fehlermeldung fragen, soweit nicht bereits bekannt. Vorhandene Autorisierungen berücksichtigen. Bei reinem Chat ohne Werkzeuge jeweils einen überschaubaren Befehlsblock geben und dessen Ausgabe abwarten. Keine erfolgreichen Aktionen erfinden.

## 1. Bestandsaufnahme
README.md, AGENTS.md, PROJECT-CONTEXT.md und die passende Datei unter docs/setup/ lesen. Checkout-Pfad, `git status --short`, Python/Tkinter, Git, SDK und JDK prüfen. Keine neue Kopie über einen bestehenden Ordner klonen und keine fremden Änderungen verwerfen.

Eine GitHub-Anmeldung (falls nötig) erfolgt über den Browser/den vorhandenen Credential Store. Keine Passwörter, Tokens oder Wiederherstellungscodes anfordern. Protokolle vor Weitergabe auf persönliche Pfade und Zugangsdaten prüfen.

## 2. Kürzesten sinnvollen Weg wählen
- Nur spielen: vorhandene, selbst gebaute oder bereitgestellte APK verwenden; keine Compiler installieren.
- APK bauen: passende Plattformanleitung abarbeiten; vorhandene Werkzeuge wiederverwenden. Fehlende Pakete gezielt einrichten.
- GUI nutzen: Python mit Tkinter, Quest-Builder-Startdatei oder direkten Python-Aufruf verwenden.

Meta-Verifizierung, Lizenzannahmen und Headset-Freigaben dem Nutzer überlassen. Offizielle Herstellerquellen verlinken; bei geänderten Vorgaben aktuelle Dokumentation prüfen. Nicht behaupten, dass eine private GitHub-Verbindung bei jedem Assistenten automatisch besteht.

## 3. Preflight und Build
`build-quest.py --check` mit den ermittelten Pfaden ausführen. Jede gemeldete fehlende Komponente korrigieren. Danach genau einen Build starten. GUI-Logs liegen in `.build/builder-*.log`.

Bei Fehlern erste relevante Ursache ermitteln, nicht nur die letzte Gradle-Zusammenfassung. Unterscheiden: Netzwerk/Anmeldung, fehlendes Submodule, SDK/JDK, CMake-Konfiguration, Kompilierung, Paketierung. Nicht ungefragt alle Abhängigkeiten aktualisieren oder die ganze Arbeitskopie löschen. Löschen eines Cache-Verzeichnisses nur gezielt und mit klarer Begründung; Benutzerdateien erhalten.

Erfolg nachweisen: Exitcode 0 und neue APK am erwarteten Pfad; bei Bedarf Zeitstempel/Hash erfassen. Eine alte APK nach Fehlermeldung zählt nicht. GUI-/CLI-Änderungen mit den Offline-Tests prüfen und bei Prozessänderungen Fehler/Abbruch zusätzlich testen.

## 4. Geräteverbindung
Meta-Konto/Team, Entwicklermodus und Debugging-Freigabe anhand der Ersteinrichtungsanleitung prüfen. `adb devices` lesen. Nur das beabsichtigte Gerät verwenden, bei mehreren Geräten Seriennummer angeben. `unauthorized` braucht Nutzerbestätigung im Headset, keine Umgehung.

## 5. Daten schützen und installieren
Vor Update laufendes Spiel speichern lassen; vorhandene Spielstände unter `/sdcard/RTTR/.s25rttr` in einen neuen lokalen Ordner sichern und Erfolg prüfen. Bestehende ausdrückliche Freigaben nicht unnötig erneut abfragen.

APK mit `quest.py install` übertragen. Signaturkonflikt: stoppen und Ursache erklären; nicht automatisch deinstallieren. Originaldaten nur aus der eigenen Gold-Installation übernehmen, nicht herunterladen oder im Repository ablegen. Bei vorhandenen Daten keinen automatischen Wechsel auf `--replace-data` vornehmen; Datenwechsel muss vom Auftrag gedeckt sein.

## 6. Abnahme und Übergabe
Über GameStartActivity starten. Nutzer im Headset Menü, Controller, Panelverstellung, Y-Zyklus und Speichern/Laden testen lassen. Nur selbst beobachtete Ergebnisse als geprüft melden. Dokumentieren: Betriebssystem, Toolchain, Quellstand, APK-Hash, ausgeführte Schritte, Ergebnis und offene Punkte. Keine Seriennummern oder vertraulichen Angaben unnötig in öffentliche Dokumentation übernehmen.
