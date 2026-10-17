# Backlog

Stand: 30.09.2026. Arbeitsbasis: Navigation `68123ef` mit gezielt wieder aufgenommenen Funktionen. Umsetzung ist keine Headset-Abnahme. Versionshistorie: [CHANGELOG.md](CHANGELOG.md); Testnachweise: [docs/VALIDATION.md](docs/VALIDATION.md).

## Als Nächstes

| ID | Priorität | Status | Aufgabe und Abnahmekriterium |
| --- | --- | --- | --- |
| VR-001 | P0 | Implementiert, Quest-Abnahme offen | Linker Grip + vor/zurück zoomt auch mit aktivem X/Y-Tausch; keine ungewollte Fensterverstellung; Zoom stoppt an seinen Grenzen. |
| VR-002 | P0 | Implementiert, Quest-Abnahme offen | Rechter Grip hält Kartenbewegung bis zum Loslassen; Cursorbewegung zieht die Karte; kein Klick erst beim Loslassen. |
| VR-003 | P0 | Implementiert, Quest-Abnahme offen | Griff am Originalrahmen verschiebt das Fenster ohne Drehung, Neigung oder Änderung seiner Wölbung; Start auf der Karte bleibt Kartenziehen, auch beim Überqueren des Randes. |
| VR-004 | P1 | Implementiert, Quest-Abnahme offen | Achsen beider Controller getrennt tauschen/umkehren; Übernehmen, Neustart und gespeicherte Einstellungen prüfen. Kartenzoom bleibt physisch vor/zurück, Y-Umkehrung gilt. |
| VR-005 | P1 | Implementiert, Quest-Abnahme offen | Ladebalken und Geduldshinweis sind in allen gewählten Auflösungen lesbar; Fortschritt entspricht Ladeschritten. |
| VR-006 | P1 | Offen | Linkshänderbedienung für Kartenziehen und Rahmenbewegung ergänzen und prüfen; aktueller Rahmenmodus setzt rechten Zeiger voraus. |
| VR-007 | P1 | Offen | Abbruch von Griffen bei Trackingverlust, Fokuswechsel, geöffnetem Dialog sowie Wiederherstellung der Fensterposition praktisch prüfen. |
| VR-015 | P1 | Implementiert, Quest-Abnahme offen | Bei Splash/Intro und Kartenladen weder Mauszeiger noch Strahl oder Zielmarkierung; keine gehaltenen Eingaben nach dem Übergang. |
| VR-016 | P1 | Implementiert, Quest-Abnahme offen | preview.3: einheitliches 40-Pixel-Raster, Controller mit Tasten/Achsen/Navigation, Fenster mit Darstellung/Position; alle Einstellungen erreichbar und ohne Überlappung. |
| VR-017 | P1 | Implementiert, Quest-Abnahme offen | Langer rechter Meta-Druck löst das erwartete Referenzraum-Ereignis aus; Breite, Abstand und Raumposition werden vollständig zurückgesetzt. |
| VR-018 | P1 | Implementiert, Quest-Abnahme offen | preview.3: höhere Tischmitte (65→40 cm), 0–90 cm frei wählbar; Übernehmen, Ansichtswechsel und Neustart prüfen. |
| VR-020 | P1 | Implementiert, Quest-Abnahme offen | preview.4: zentrierter Leinwandstart unabhängig von gespeicherter Ansicht/Position; Option „Trigger halten: Karte greifen“ (Tippen = Klick, Bewegen = Karte ziehen, vor/zurück = Zoomstufe). Prüfen: Klickgenauigkeit, Doppelklick, Gebäude setzen, Straßenbau, Tischansicht, Linkshänder. |
| VR-021 | P1 | Implementiert, Quest-Abnahme offen | Tisch-Simulator im VR-Schnellmenü: Neigung, Höhe und Abstand der Tischansicht mit wenigen Klicks durchschalten, sofort sichtbar, ohne Übernehmen-Dialog. |
| VR-022 | P1 | Stufe 1+2 implementiert (preview.9), Quest-Abnahme offen; Stufe 3 geplant, siehe docs/PLAN-Tisch-und-Diorama.md | Tisch auf echte Fläche legen: Raumscan/Szenendaten der Quest (Scene API, Tisch-Anker) nutzen, damit das Spielbrett auf dem realen Schreibtisch liegt – wie in einem fremden Generals-Port gesehen. |
| VR-023 | P2 | Geplant, siehe docs/PLAN-Tisch-und-Diorama.md | Diorama-Wirkung verbessern: Geländehöhe wirkt auf dem geneigten 2D-Brett noch flach. Optionen prüfen (echter 3D-Geländepass aus `MapNode::altitude`, Parallaxe/Höhenversatz je Kachel, stärkere Schattierung); siehe docs/wiki/Diorama-und-Ansichten.md. |
| VR-019 | P2 | Implementiert, Quest-Abnahme offen | Gebäudehilfe auswählbar (Aus / vier Websites); Browseröffnung, Rückkehr zum Spiel und gespeicherte Auswahl prüfen. |
| VR-024 | P1 | Implementiert (Zweig xr-menu-pflicht, nach 2.1.0-preview.13) – Quest-Abnahme offen | VR-Menü-Pflichtumfang (Skill xr-vr-menu §0): „Spiel beenden“ im Schnellmenü mit sichtbarem Scharf-Zustand (2. Klick in 3 s, Doppelpuls, speichert Einstellungen + Auto-Speicherstand), Strahl/Zielpunkt „Aus“ nur auf der freien Karte (persistent), Schnellmenü zweistufig mit 8 großen Knöpfen, Zentrieren/Leinwand-Tisch je einmal, Passthrough ausgegraut ohne Laufzeit-Unterstützung, Stick-Fokus/A/B in allen VR-Fenstern mit Halten-Wiederholung. Prüfen: Beenden einmal/zweimal/abwarten, Strahl Aus auf Karte vs. im Menü, Fokusrahmen, A/B schließen ohne Nachwirkung auf Spielaktionen. |

## Später, einzeln wieder aufnehmen

| ID | Priorität | Status | Aufgabe und Abnahmekriterium |
| --- | --- | --- | --- |
| VR-010 | P1 | Zurückgestellt | Aufrechte Menüs in der Tischansicht: vollständige Schrift und Flächen, korrekte Strahltreffer, keine schwarzen Rechtecke. Frühere separate Menü-Layer hatten Regressionen. |
| VR-011 | P1 | Implementiert, Quest-Abnahme offen | Passthrough-Schalter im Favoritenmenü und 0–100-%-Regler in 10-%-Stufen prüfen; Fenster bleibt deckend. Auf ausdrücklichen Nutzerwunsch in preview.2 wieder aufgenommen. |
| VR-012 | P2 | Beta implementiert, Quest-Abnahme offen | preview.3: optionales externes HQ-Modell auf der Tischkarte; Position, Zoom, Tiefendarstellung, Menüwechsel und Leistung praktisch prüfen. Weitere Gebäude offen. Nutzerassets außerhalb des Repositories. |
| VR-013 | P2 | Offen | Bildqualität und Leistung in hohen Auflösungen messen; FPS-Anzeige allein ist keine Abnahme. |
| VR-014 | P2 | Offen | Alle Texteingaben, Speichern/Laden und automatische Dateinamen vollständig auf Quest durchspielen. |
| SETUP-001 | P2 | Teilweise | CLI-Build unter Windows 10 am 27.09.2026 erfolgreich (siehe VALIDATION.md). gettext-Prüfung (`msgfmt`, `msgmerge`) im Preflight von `build-quest.py --check` am 30.09.2026 implementiert (Fehler mit Installationshinweis, da der Engine-Build gettext zwingend braucht; gebaut, Unit-Tests grün). Am 30.09.2026 zeigt der grafische Builder das gettext-Ergebnis zusätzlich als eigene Statuszeile (OK bzw. fehlendes Werkzeug mit Installationshinweis, ohne Pfade); Nachbesserung: Kindprozess mit `PYTHONIOENCODING=utf-8`, damit der Fehlerfall unter Windows nicht an cp1252 scheitert (gebaut, 9 Python-Tests grün inkl. Fehlerfall über die Prozessgrenze, GUI nur per `py_compile` geprüft). Fehlerfall am echten Builder-Aufrufweg nachgeprüft am 30.09.2026: Aufruf wie `quest-builder.py` (`sys.executable -u build-quest.py --sdk … --java … --check`, `env=child_process_env()`, stdout+stderr als UTF-8 mit `errors="replace"`, jede Zeile durch `gettext_status()`), Windows 10 ohne `PYTHONUTF8`/`PYTHONIOENCODING` im Elternprozess: ohne gettext im PATH exit 1, kein U+FFFD, Umlaute im Protokoll erhalten, Statuszeile „gettext fehlt: msgfmt, msgmerge – Installationshinweis: Windows: winget install --id mlocati.GetText (danach Builder neu öffnen)“ (ohne Pfad); Gegenprobe mit vollem PATH exit 0, kein U+FFFD, Statuszeile „gettext: OK“. Kein Abweichen vom getesteten Weg, kein Fix nötig; 9 Python-Tests grün, `py_compile` fehlerfrei. Offen: Linux-Build. |
| REPO-001 | P1 | Erledigt | Branch `fix/vr-baseline-20260926` am 27.09.2026 per PR #1 nach `main` übernommen (Konflikte zugunsten des Baseline-Stands). Offen: `main` auf GitHub schützen. |
| SETUP-002 | P2 | Offen | Benutzerfreundlichen Import eigener legaler Spieldaten und Editionen-Kompatibilität prüfen. |
| DOC-001 | P2 | Erledigt | `STATUS.md` mit Rückroll-Tag `v2.1.0-preview.8` und Stand gebaut/installiert/bestätigt je Preview (nur aus CHANGELOG/VALIDATION belegt); Leerzeilen in der Tabelle „Als Nächstes“ entfernt; README verweist auf STATUS.md. |
| DOC-002 | P2 | Erledigt | Doku-Bündel aus der QS: README auf Version 2.1.0-preview.13; CHANGELOG „Unveröffentlicht“ mit eigenem Nachweis je Eintrag statt Sammelsatz; STATUS.md preview.10 „installiert“ = „unklar“ mit Begründung. |
| DOC-003 | P2 | Erledigt | README/Wiki: Linkshändermodus gestrichen, solange VR-006 offen. |

## Arbeitsregeln

1. Eine überschaubare Änderung umsetzen und im Updatelog erfassen.
2. Passende Tests ausführen und APK bauen; exakten Versionsstand notieren.
3. Vor Installation Benutzerdaten sichern. Gebaut, installiert und im Headset bestätigt getrennt dokumentieren.
4. Erst nach Rückmeldung den betreffenden Eintrag als bestätigt markieren und den nächsten Schritt beginnen.
5. Jede verteilte APK erhält eine neue Versionsnummer und einen höheren Android-versionCode; frühere APKs nicht überschreiben.
