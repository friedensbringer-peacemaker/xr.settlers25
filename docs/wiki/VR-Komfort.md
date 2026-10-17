# VR-Komfort, Tasten und Favoriten

**Linke Menütaste → Komfort / Belegung** öffnet die zusätzlichen VR-Optionen. Alternativ: **VR: Controller / Bild → Weitere VR-Optionen**. Sechs Seiten halten die Einstellungen auch bei großer GUI bedienbar. **Übernehmen** speichert die Auswahl; **Abbrechen** verwirft nicht übernommene Komfortänderungen. Der Seitenwechsel behält die Auswahl zunächst im Dialog.

## Komfort

| Einstellung | Auswahl und Wirkung |
| --- | --- |
| Zeigerglättung | Aus, Leicht, Stärker. Glättet kleine Bewegungen zeitabhängig, statt die Empfindlichkeit der Karte zu ändern. Standard: Leicht. |
| Klick stabilisieren | Ein/Aus. Hält den Zielpunkt zu Beginn des Triggerdrucks etwa 85 ms stabil; danach bleiben Ziehen und Verschieben möglich. |
| Klick-Vibration | Ein/Aus. Kurzer, leichter Impuls bei einem Triggerklick auf das Spielfenster. Bestätigt die Eingabe, nicht den Erfolg eines Spielbefehls. |
| Schaltflächen hervorheben | Ein/Aus. Zusätzliche warme Hervorhebung der Spielschaltflächen beim Darüberzeigen. Die normale Spiel-Hoveranzeige bleibt erhalten. |
| Zeigerhand | Rechts oder Links. Trigger und Griff der gewählten Hand dienen als Maus. Die physischen X/Y/A/B-Tasten behalten ihre eigene Belegung. Kartenziehen und Rahmenbewegung mit linkem Zeiger sind noch nicht umgesetzt (BACKLOG VR-006). |
| Listen mit Stick scrollen | Ein/Aus. Der rechte Stick scrollt Listen, Tabellen und Scrollleisten unter dem Zeiger vertikal. Auch am Listenende wird nicht auf Kartenbewegung umgeschaltet. Außerhalb dieser Elemente bleibt die Kartensteuerung erhalten. |
| Pause bei Fokusverlust | Aus oder Einzelspieler. Beim Absetzen bzw. Wechsel in die Quest-Systemoberfläche wird eine laufende Einzelspielerpartie pausiert. Beim Zurückkehren verlangt „Ja, weiterspielen“ eine Bestätigung. „Pausiert lassen“ schließt nur die Rückfrage. Mehrspieler wird nicht automatisch pausiert. |
| Fenstergröße merken | Ein/Aus. Speichert die Breite; Position und Ausrichtung werden bei jedem Start neu vor dir zentriert. Mit „Letzten Abstand merken“ wird der zuletzt eingestellte Abstand separat für Leinwand und Tisch gespeichert. |

**Fenster zentrieren** (VR-Schnellmenü, einziger Ort dieser Schaltfläche) richtet die gewählte Ansicht vor der aktuellen Kopfposition und horizontalen Blickrichtung aus. Bei der Tischansicht bleiben Neigung und Höhenversatz erhalten. Die zur Ansicht gehörende Startdistanz wird verwendet. Diese Aktion wirkt sofort. Der Druck auf den linken Stick zentriert ebenfalls und setzt die Breite auf 2 m zurück, sofern die Fensterverstellung freigegeben ist.

Die gespeicherte Position bezieht sich auf den lokalen Trackingraum. Nach einer geänderten Quest-Raumkalibrierung gegebenenfalls erneut zentrieren. Die neue Wahl einer Startdistanz löscht den gemerkten Abstand dieser Ansicht; nach Zentrieren gilt die neue Auswahl.

Bei Fokusverlust, Tracking-Ausfall oder Wechsel der Zeigerhand werden gehaltene Mausaktionen gelöst. Ein durchgehend gehaltener Controllerknopf muss nach Fokusverlust zuerst losgelassen werden, bevor er erneut eine Aktion auslöst.

## Tasten

X links, Y links, A rechts und B rechts lassen sich jeweils für **kurzen** und **langen** Druck belegen. Auswahl: Aus, Kartenzoom, Baumenü, Spielmenü/Transport, Geschwindigkeit, Speichern, Pause/Weiter, Minikarte, Hauptquartier, Bauhilfe, Inventar, Nachrichten, Chat, Gebäudenamen, Produktivität, Fenster zentrieren, VR-Schnellmenü und Ansicht wechseln.

Standard kurz: **X Zoom, Y Geschwindigkeit, A Baumenü, B Spielmenü**. Standard lang: Aus. Ist eine lange Aktion gewählt, wird sie nach **0,6 Sekunden** ausgelöst; die kurze Aktion folgt dann erst beim Loslassen vor dieser Schwelle. Ein langer Druck löst nicht zusätzlich die kurze Aktion aus. Die linke Menütaste bleibt als verlässlicher Zugang zum VR-Menü fest belegt.

Spielaktionen sind außerhalb einer Partie und in modalen Dialogen gesperrt. Das Baumenü bleibt kontextabhängig und erfordert bei Controller-Auslösung einen Treffer auf der Bildfläche. Es verändert keine begonnene Straßenbauaktion.

## Favoriten

Sechs frei wählbare Aktionen bilden die erste Seite des VR-Schnellmenüs, darunter acht große Knöpfe: Weiter, Fenster zentrieren, Leinwand/Tisch, Passthrough, VR-Einstellungen, Spiel-Einstellungen, „Werkzeuge / alle Aktionen“ und Spiel beenden (zweimal klicken: der erste Klick färbt den Knopf um und zeigt „Sicher? Nochmal klicken“, nach 3 s fällt er zurück; vor dem Beenden werden Einstellungen und – im laufenden Spiel – der Auto-Speicherstand gesichert). „Werkzeuge / alle Aktionen“ zeigt die umfangreiche Auswahl sowie GUI-Größe, Tisch-Simulator, Screenshot-Modus, Video-Aufnahme und Passthrough-Stärke. In allen VR-Fenstern bewegt der rechte Stick einen gelben Fokusrahmen (Halten wiederholt), A löst aus bzw. stellt eine Auswahlliste eine Stufe weiter (links/rechts ebenso), B schließt das Fenster. Mit „Aus“ lässt sich ein Favoritenplatz deaktivieren. Favoriten ändern keine Controllerbelegung.

## Grafikprofile und sichere Auflösungswechsel

| Profil | Auflösung | GUI |
| --- | --- | --- |
| Beste Leistung | 1600 × 1000 | 160 % |
| Ausgewogen | 1920 × 1200 | 200 % |
| Beste Lesbarkeit | 3840 × 2400 | 400 % |

Die Profile ändern Auflösung und GUI gemeinsam. Persönliche Helligkeit, Kontrast, Bildschärfe und Wölbung bleiben erhalten. Nicht vom Gerät angebotene Profilauflösungen sind deaktiviert. Die Profilnamen beschreiben das Ziel; die tatsächliche Leistung muss im Headset mit einer großen Karte geprüft werden.

Nach einem Auflösungswechsel bleiben **15 Sekunden** für „Behalten“. „Zurück“ oder der Ablauf des Timers stellt Auflösung und GUI wieder her. Auch Änderungen in den normalen Grafikoptionen werden so abgesichert. Während der Bestätigung wird weiterhin die vorherige Auflösung gespeichert, damit ein App-Abbruch keine unbestätigte Einstellung zum nächsten Start übernimmt.

Praktische Headset-Abnahme siehe [Prüfstand](../VALIDATION.md).

## Ansicht: Leinwand und Spielbrett

**Linke Menütaste → Komfort / Belegung → Ansicht**:

| Einstellung | Auswahl |
| --- | --- |
| Darstellung | Leinwand (Standard) oder Tischansicht |
| Neigung des Spielbretts | 30, 45 (Standard), 60 oder 75 Grad gegenüber der aufrechten Leinwand |
| Unter der Blickhöhe | 40, 65 (Standard) oder 90 cm unter der beim Zentrieren erfassten Kopfposition |
| Tisch-Startabstand | 0,8 m, 1,2 m (Standard) oder 1,8 m |

**Übernehmen** aktiviert und speichert die Auswahl. Die Tischansicht ist ein geneigtes, flaches Spielbrett mit der originalen **2D-Spielansicht**, noch kein dreidimensionales Gelände. Menüs und Tastatur liegen zunächst auf derselben Fläche. Die Leinwand behält ihre eigene Wölbung und Startdistanz. Umschalten lädt die laufende Partie nicht neu.

Im Schnellmenü wechselt der große untere Button direkt zwischen Tischansicht und Leinwand. Alternativ lässt sich **Ansicht wechseln** als Favorit oder kurzer/langer X/Y/A/B-Druck belegen. Die bisherigen Standardbelegungen bleiben erhalten. Für einen schnellen Rückweg empfiehlt sich beispielsweise X lang.

Der Tisch-Startabstand wird beim nächsten Zentrieren, Wechsel zur Tischansicht oder App-Start benutzt, sofern kein gemerkter manueller Abstand vorliegt. Der linke Stick verändert weiterhin Breite und Abstand. Neigung und Höhe werden um die Bildmitte berechnet; Strahl und Klickposition folgen derselben Transformation. Beim Ändern der Darstellung, Neigung oder Höhe werden gehaltene Klicks gelöst; die Knöpfe müssen erst losgelassen werden.

Entwicklungsplan für echtes Gelände: [Diorama und Ansichten](Diorama-und-Ansichten.md).

## Sticks und Startposition

**Linke Menütaste → Komfort / Belegung → Sticks**:

| Einstellung | Wirkung |
| --- | --- |
| Rechte Y-Achse | Normal oder umgekehrt; gilt für Karte und Listen. |
| Linker Stick: Fenster | Immer / Griff der anderen Hand halten (Standard) / Gesperrt. Bei rechter Zeigerhand den linken Griff halten, bei linker Zeigerhand den rechten. So kollidiert die Freigabe nicht mit dem Maus-Rechtsklick. Auch der linke Stickdruck zum Zurücksetzen benötigt diese Freigabe. |
| Kartentempo | 1×, 2× (Standard), 4×. Analoge, zeitabhängige Bewegung statt einzelner Pfeiltastenschritte; diagonale Bewegung ist nicht schneller als gerade Bewegung. |
| Karten-Turbo | Aus / rechten Stick gedrückt halten (Standard) / Doppeltippen und halten / beide. Turbo verdreifacht das gewählte Tempo. Beim Doppeltippen innerhalb von 300 ms nach dem Loslassen erneut in dieselbe Richtung drücken und halten. |
| Trigger halten: Karte greifen | Aus (Standard) / An. Trigger der Zeigerhand auf der freien Karte halten und den Strahl bewegen: Die Karte hängt am Strahl (links/rechts/oben/unten verschiebt). Hand bei gehaltenem Trigger 6 cm zum Fenster hin bewegen zoomt eine Stufe hinein, 6 cm zurück eine Stufe heraus; jede Stufe vibriert kurz. Kurzes Tippen ohne Bewegung bleibt ein Linksklick, der beim Loslassen ausgelöst wird. Über Fenstern, Bedienleiste und Dialogen verhält sich der Trigger normal. Zoom per linkem Griff + Stick und X bleibt unverändert. |
| Start vor Blickrichtung | Keine Option mehr, immer aktiv: Jeder App-Start beginnt mit der Leinwand, zentriert vor der aktuellen Kopfposition, sobald gültiges Tracking vorliegt (auch vor dem Eingabefokus). Eine zuletzt gewählte Tischansicht oder seitlich verschobene Position wird nicht übernommen. |
| Letzten Abstand merken | Standard An. Merkt manuelle Abstände separat für Tisch und Leinwand. Vor dem ersten Verstellen gelten die vorhandenen Startdistanz-Presets. Änderungen werden nach kurzer Ruhe gespeichert. |

Die Fensterbreite wird bei neutralem linken Stick nicht mehr automatisch verkleinert. Turbo beschleunigt keine Listen und wirkt nicht hinter modalen Menüs. Zeigt der Mauszeiger auf ein geöffnetes Spielfenster, bleibt die Karte stehen. Für große Sprünge **Minikarte** im Schnellmenü öffnen und die gewünschte Kartenstelle anklicken. Trigger und Maus-Griff bleiben normale Spielaktionen.

## Tisch-Simulator

**Linke Menütaste → Tisch-Simulator** öffnet ein kleines Fenster, dessen Änderungen sofort wirken und gespeichert werden – ohne „Übernehmen“. Das Fenster bleibt offen, damit man den Tisch dahinter direkt vergleichen kann.

| Voreinstellung | Neigung | Tischmitte unter Blickhöhe | Abstand |
| --- | --- | --- | --- |
| Schreibtisch | 45° | 40 cm | 0,8 m |
| Couchtisch | 30° | 70 cm | 1,2 m |
| Stehpult | 60° | 20 cm | 0,8 m |
| Großer Tisch | 30° | 50 cm | 1,8 m |

Darunter lassen sich Neigung (30–75°), Tischmitte (0–90 cm, 10-cm-Schritte) und Abstand (0,8/1,2/1,8 m) mit −/+ einzeln verstellen. Aus der Leinwand wird dabei automatisch auf die Tischansicht gewechselt; ein neuer Abstand rückt das Brett wieder vor dich.

## Screenshot-Modus

**Linke Menütaste → Screenshot-Modus** (Favoritenseite, neben Passthrough). Solange kein Fenster offen ist, sind Strahl, Zielring und Mauszeiger unsichtbar; Klicken und Scrollen funktionieren weiter. Der kurze Fokusverlust durch die Quest-Screenshot-Kombination (Meta-Taste + Trigger) pausiert das Spiel nicht und löst keine Weiterspielen-Frage aus. Zum Beenden das Schnellmenü öffnen – dort ist der Strahl wieder sichtbar – und „Screenshot-Modus beenden“ wählen. Der Modus wird nicht gespeichert.

Das kleine Systemfenster „App-Name nicht verfügbar / Beenden“ gehört zur Quest-Oberfläche und erscheint bei der Meta-Taste unabhängig vom Spiel.

## Video-Aufnahme

**Linke Menütaste → Video aufnehmen (Spielbild + Ton)** startet eine Aufnahme des Spielbilds mit Spielton; derselbe Knopf („Aufnahme beenden“) stoppt sie. Die Datei liegt danach unter `/sdcard/RTTR/videos/` als `xr.rttr-Datum-Uhrzeit.mp4`. Aufgenommen wird das flache Spielbild – scharf, ohne Kopfbewegungen, Zimmer oder Controllerstrahl. Für den räumlichen VR-Eindruck weiterhin die Aufnahmefunktion der Quest verwenden. Ein roter Punkt am Spielfenster zeigt die laufende Aufnahme an und ist im Video nicht zu sehen. Für YouTube die Musik vorher ausschalten (Einstellungen → Sound/Musik).

## Vorgeschlagene Spielstandnamen

Im Speichern-Dialog ist **„Namen vorschlagen: Zeitstempel - Spieler - Karte“** standardmäßig aktiviert. Beispiel: `2026-09-26_19-30-00-Spieler-Europa.sav`. Verwendet werden lokale Quest-Zeit inklusive Sekunden, aktueller Spielername und Kartentitel.

Der Haken wird gespeichert. Der Vorschlag erscheint beim Öffnen und nach dem Speichern erneut. Das Textfeld bleibt bearbeitbar und öffnet die VR-Tastatur. Vorhandene automatisch vorgeschlagene Dateinamen erhalten einen Nummernzusatz; ungültige Dateinamenzeichen werden ersetzt. Ein bewusst manuell eingegebener oder aus der Liste gewählter Name bleibt möglich. Der Haken verändert nicht das zeitgesteuerte Auto-Save-Intervall.

## Ergänzungen ab 2.1.0-preview.2

Im **Favoritenmenü** lässt sich Passthrough direkt schalten. Der Regler **Umgebung** arbeitet in 10-%-Schritten: 0 % schaltet den Hintergrund aus, 100 % zeigt ihn vollständig. Die Spielfläche bleibt deckend. Zusätzlich ist „Passthrough an/aus“ als Favoriten- und Tastenaktion auswählbar.

Der Befehl **Fenster zentrieren** setzt die Breite auf 2 m und verwendet den ausgewählten Startabstand. Die App reagiert auch auf die systemweite Änderung ihres OpenXR-Referenzraums, wie sie beim Neuzentrieren der Quest erwartet wird. Die Auslösung über langen rechten Meta-Druck ist noch praktisch zu prüfen.

Der Achsentausch ist nur noch unter **Komfort → Achsen** einstellbar. Während der Lade-Desktops sind Mauszeiger und XR-Strahl ausgeblendet.

## Menüstruktur ab 2.1.0-preview.3

- **Controller:** Tasten, Achsen und Navigation.
- **Fenster:** Darstellung/Tisch und Position/Abstand.
- **Komfort:** Zeigerhand, Glättung, Vibration, Fokuspause und Klickhilfen.
- **Favoriten:** Belegung der sechs Schnellaktionen.
- **Grafik:** Auflösungsprofile. Das separate Bildmenü besitzt die Bereiche Bild/Fenster und Strahl/Zeiger.

Unter **Fenster → Darstellung/Tisch → Tischmitte unter Blickhöhe** gilt: kleinere Zahl = höherer Tisch. 0 cm liegt auf Blickhöhe, 40 cm ist der neue Standard, 90 cm die tiefste Einstellung. Alle zehn Werte sind unabhängig von Neigung und Startabstand wählbar. Höhe/Neigung gelten nach Übernehmen; der Startabstand nach Zentrieren oder Ansichtswechsel. Die Höhe bezieht sich auf die Tischmitte und die beim Zentrieren ermittelte Kopfposition.

## Online-Gebäudehilfe

Unter **Komfort → Online-Gebäudehilfe** wählst du **Aus** (Standard) oder:

- [settlers2.net – Bauanleitung](https://settlers2.net/guides/guide-to-buildings/)
- [settlers2.net – Datenblatt](https://settlers2.net/guides/datasheet-for-buildings/)
- [SiedlerCommunity](https://www.siedlercommunity.de/siedler2/gebaeude/)
- [JH-ISCH – Gebäudetipps](https://jh-isch.de/s2/tipps/t_gebaude.php)

Nach **Übernehmen** erscheint im Infofenster eines Gebäudes der Button **Online-Gebäudehilfe öffnen**. Er öffnet die ausgewählte Übersichtsseite im Quest-Browser. Auch Baustellen, Lager und Militärgebäude sind angebunden. Aus blendet den Button aus. Internet ist nur für die Website erforderlich. Die Pause bei Fokusverlust richtet sich weiterhin nach deiner Komfort-Einstellung; Browseröffnung und Rückkehr müssen noch praktisch auf Quest geprüft werden.

## Beta: 3D-Hauptquartier

Unter **Fenster → Darstellung / Tisch → Beta: 3D-Hauptquartier** lässt sich das HQ-Modell einschalten. Standard ist **Aus**. Die Funktion gilt nur für die Tischansicht: Das Modell ersetzt das flache HQ an dessen Kartenposition und folgt Kartenbewegung und Zoom. Der Klick erfolgt weiterhin auf der ursprünglichen Gebäudeposition, nicht auf einer neuen 3D-Kollisionshülle.

Bei geöffneten Spielmenüs und am Bildrand wird die Originalgrafik verwendet. Fehlt das Modell, bleibt das HQ ebenfalls sichtbar. Die Modelltextur besitzt ihre eigene blaue Flagge; Spielerfarben werden noch nicht dynamisch ersetzt. Überlagerungen mit Nachbargebäuden und die Leistung sind Beta-Prüfpunkte.

Die private optimierte Assetdatei muss unter `/sdcard/RTTR/vr-assets/headquarters.xrhq` liegen. Sie ist auf dem Entwicklungsgerät vorhanden, wird aber weder in GitHub noch in die APK aufgenommen. Die Beta verändert keine Spielstände und aktiviert keine der früher zurückgebauten separaten Menü-Layer.
