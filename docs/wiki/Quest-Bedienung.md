# Quest-Bedienung

Stand: 26. September 2026. Belegung des aktuellen Entwicklungsbuilds.

| Eingabe | Funktion |
| --- | --- |
| Gewählte Zeigerhand | Auf das Spielfenster zeigen; Standard rechts, links in VR-Komfort wählbar |
| Trigger der Zeigerhand | Linksklick am Zielpunkt. Optional „Trigger halten: Karte greifen“: auf der freien Karte halten und Strahl bewegen verschiebt die Karte, Hand vor/zurück zoomt; kurzes Tippen bleibt ein Klick |
| Griff der Zeigerhand | Rechtsklick am Zielpunkt |
| Linkes X | Kartenzoom wie Z: nächsthöhere Stufe, nach der größten wieder kleinste |
| Rechtes A | Kontext-/Baumenü am anvisierten Kartenpunkt; auf einem Gebäude dessen Fenster |
| Rechtes B | Spielmenü, unter anderem Transportprioritäten |
| Linke Menütaste | VR-Schnellmenü mit direkt auswählbaren Tastenkürzeln |
| Rechter Stick | Karte verschieben; über Listen gezielt scrollen, wenn aktiviert |
| Linker Stick horizontal | Spielfenster verstellen, standardmäßig nur mit Griff-Freigabe; Achsen sind tauschbar |
| Linker Stick nach hinten/vorne | Zweite Fensterachse, ebenfalls mit Griff-Freigabe |
| Linken Stick drücken | Mit Griff-Freigabe: Breite auf 2 m, zentrieren und gemerkten bzw. gewählten Abstand verwenden |
| Linkes Y | Im Einzelspiel Beschleunigung durchschalten und wieder ausschalten |

## Spielgeschwindigkeit
Jeder neue Druck auf Y schaltet weiter:

**Normal → 1,25× → ca. 1,67× → 2,5× → 5× → Normal.**

Die Faktoren entsprechen den angeforderten Simulationsschrittzeiten 50/40/30/20/10 ms. Tatsächlich erreichbare Geschwindigkeit hängt von der Rechenleistung ab. Gedrückthalten wiederholt nicht. Auf das Panel zu zeigen ist dafür nicht erforderlich. Y erzeugt kein Zeichen in Textfeldern. Diese Funktion ändert im Mehrspielermodus keine Geschwindigkeit.

## Spielfenster
Der Abstand ist zwischen 0,7 und 3,5 m einstellbar. Die Breite wird zusätzlich abhängig vom Abstand begrenzt; der maximale horizontale Sichtwinkel am Referenzursprung beträgt etwa 84 Grad. Das Heranholen des gesamten Fensters ist vom Kartenzoom getrennt.

Die Startdistanz ist im VR-Menü wählbar: Nah (1,2 m), Mittel (1,8 m) oder Weit (2,8 m). Nach Übernehmen gilt sie beim nächsten Start und beim Stick-Reset. Der Reset setzt Größe und Abstand zurück und richtet das Fenster vor der aktuellen horizontalen Blickrichtung aus. „Letzten Abstand merken“ speichert manuelle Abstände getrennt für Leinwand und Tisch. Eine neue Startdistanz-Auswahl setzt diesen gemerkten Abstand zurück.

## Texteingabe
Ein Klick auf ein bearbeitbares Textfeld öffnet die gemeinsame VR-Tastatur: beispielsweise Spielername, Speichername, Chat, Passwort oder Zahlenfeld. Sie bietet QWERTZ, Umlaute, Zahlen, Sonderzeichen, Groß-/Kleinschreibung, Leerzeichen, Rücktaste, Cursorbewegung und Leeren. Passwortfelder bleiben maskiert; die Zeichen- und Längenregeln des ursprünglichen Feldes gelten weiterhin.

**Übernehmen** überträgt den Text. **Enter / Absenden** löst zusätzlich die Enter-Aktion des Feldes aus. **Abbrechen** verwirft die Eingabe. Anschließend gegebenenfalls die normale Speichern-/Bestätigen-Schaltfläche benutzen. Dies ist eine integrierte Spieltastatur, keine Meta-Systemtastatur.

## VR-Schnellmenü
Die linke Menütaste öffnet große Schaltflächen für Speichern, Pause, Minikarte, Hauptquartier, Bauhilfe, Gebäudenamen, Produktivität, Inventar, Nachrichten, Chat, Geschwindigkeit, Zoom, Spielmenü und Hilfe. Außerhalb einer laufenden Partie sind Spielaktionen deaktiviert. Zusätzlich erreichbar: VR-Bildoptionen, normale Einstellungen und GUI-Größe in 10-Prozent-Schritten. In Bestätigungsdialogen öffnet sich kein zweites Schnellmenü; in VR-Dialogen schließt die Menütaste den Dialog.

X/A/B erzeugen keine Buchstaben in Textfeldern. In modalen Dialogen bleiben diese Spielaktionen gesperrt. A erfordert ein Ziel auf dem Spielfenster und eine aktive Karte; während Straßenbau wird keine weitere Bauaktion ausgelöst.

## Individuelle VR-Optionen
Unter **Einstellungen → VR: Controller / Bild** lassen sich die linken Stickachsen tauschen, Bildschärfe, Helligkeit/Kontrast und Fensterwölbung ändern sowie Strahl, Zielpunkt und Maus-Icon einstellen. [Alle Optionen und Vorschau/Speichern](VR-Optionen.md).

## Zusätzliche Komfortoptionen
Die linke Menütaste zeigt zuerst sechs konfigurierbare Favoriten. **Komfort / Belegung** öffnet Ein/Aus-Schalter für Bedienhilfen, Zeigerhand, Kurz-/Langdruckbelegung, Favoriten und sichere Grafikprofile. [Alle Komfortoptionen](VR-Komfort.md).

### Schutz vor versehentlichem Zoomen und schnelleres Navigieren

Ab der Navigations-Vorabversion wird der linke Stick standardmäßig mit dem **Griff der anderen Hand** freigegeben: bei rechter Zeigerhand den linken Griff halten, dann den linken Stick bewegen. Ohne Freigabe bleiben Fenstergröße, Abstand und Stickdruck-Reset unverändert. Zentrieren über das Schnellmenü bleibt verfügbar.

Unter **Komfort / Belegung → Sticks** lassen sich dieser Schutz, die rechte Y-Achse, das Kartentempo, Turbo und der Start einstellen. Standard: 2× Kartentempo; rechten Stick gedrückt halten für zusätzlich 3× Turbo. Doppeltipp-Turbo ist wählbar. Im Speichern-Dialog gibt es einen dauerhaft merkbaren Haken für **Zeitstempel–Spieler–Karte** als Namensvorschlag. Details: [VR-Komfort](VR-Komfort.md#sticks-und-startposition).
