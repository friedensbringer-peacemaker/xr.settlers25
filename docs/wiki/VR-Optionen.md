# VR: Controller und Bild

Öffnen: **Linke Menütaste → VR: Controller / Bild** oder **Hauptmenü → Einstellungen → VR: Controller / Bild**. Im laufenden Spiel: **Einstellungen → VR: Controller / Bild**.

Bild- und Controlleränderungen werden sofort als Vorschau angezeigt. Die Startdistanz gilt beim nächsten App-Start oder beim Drücken des linken Sticks (Fenster-Reset). **Übernehmen** speichert sie dauerhaft; **Abbrechen** oder das Schließen des Fensters verwirft die Vorschau. **Standardwerte** setzt alle Optionen zunächst in der Vorschau zurück; erst Übernehmen speichert den Reset.

| Option | Auswahl / Wirkung |
| --- | --- |
| Linker Stick: X/Y tauschen | Vertauscht Fensterbreite und Abstand. Ohne Tausch: links/rechts ändert die Breite, vor/zurück den Abstand. Mit Tausch: vor/zurück ändert die Breite, links/rechts den Abstand. Die X-/Y-Tasten sind davon unabhängig; Y schaltet weiterhin Spielgeschwindigkeiten durch. |
| Startdistanz | Nah (1,2 m), Mittel (1,8 m, Standard), Weit (2,8 m). Übernehmen speichert die Auswahl für Start und Fenster-Reset. |
| Helligkeit | 50–150 %, Schritte von 10 %. 100 % ist unverändert. |
| Bildschärfe | Weich, leicht weich, neutral, leicht scharf, scharf. Vorschau sofort; Standard neutral. |
| Kontrast | 50–150 %, Schritte von 10 %. 100 % ist unverändert. |
| Fensterwölbung | Flach, leicht gewölbt (45°), stärker gewölbt (75°). Die Fenstermitte bleibt am gleichen Ort. |
| Strahl | Dünn, mittel, dick oder **Aus**. Aus gilt nur auf der freien Karte – in Menüs und Dialogen bleibt der Strahl sichtbar; die gewählte Stärke bleibt gespeichert. |
| Strahlfarbe | Weiß/hell, Hellgrau, Dunkelgrau, Warmgelb. |
| Zielpunkt | Punkt, Ring, Kreuz oder **Aus** (wie beim Strahl nur auf der freien Karte). |
| Zielpunktgröße | Klein, mittel, groß. |
| Maus-Icon | **Aus** blendet das originale Spielsymbol vollständig aus; alternativ 75 %, 100 %, 150 % oder 200 % Größe. Controllerstrahl, Zielpunkt und Klickfunktion bleiben erhalten. |
| Zeiger waagerecht / senkrecht | Verschiebt nur den Spiel-Mauszeiger gegenüber dem Strahl, falls er nicht genau unter dem Zielpunkt liegt: Regler mit ‹ › in Schritten von 0,25 % der Bildbreite bzw. -höhe, höchstens ±15 % (Anzeige z. B. „+1,25 %“ oder „0“). ‹ = links/oben, › = rechts/unten; mit Stick-Fokus wiederholt Halten. Wirkt sofort als Vorschau, gilt nicht in den VR-Menüs selbst und wird mit den VR-Einstellungen gespeichert. |

Bei einem zu hellen Bild zuerst Helligkeit 80–90 % ausprobieren. Helligkeit und Kontrast betreffen nur das Spielfenster, nicht die globale Quest-Displayhelligkeit oder den separat gezeichneten Controllerstrahl. Extreme Kontrastwerte können Details in sehr hellen/dunklen Flächen abschneiden. Die Auflösung wird durch diese Regler nicht verändert.

Die Wölbung verwendet `XR_KHR_composition_layer_cylinder`, sofern die Laufzeit diese Erweiterung anbietet; andernfalls bleibt die Darstellung flach. Controller-Treffer werden auf derselben Zylindergeometrie berechnet. Originalauflösung und Compositor-Layer bleiben erhalten. [Khronos: Cylinder-Layer](https://registry.khronos.org/OpenXR/specs/1.1/man/html/XrCompositionLayerCylinderKHR.html).

Die Bildanpassung verwendet einen einfachen GLSL-Shader über gl4es. Falls der Shader auf einer Laufzeit nicht kompiliert, bleibt das originale Bild sichtbar und der Fehler wird mit `[videoOpenXR]` protokolliert. [GL4ES](https://github.com/ptitSeb/gl4es).

Die Werte liegen in den bestehenden RTTR-Einstellungen unter `[video]` (`vr_*`). Persönliche Einstellungsdateien gehören nicht ins Repository.

## Standardwerte
Das Standardprofil entspricht dem gewählten Quest-Screenshot: Achsentausch an, Helligkeit 80 %, Kontrast 120 %, leichte Wölbung, mittlerer weißer Strahl, kleiner Ring und Maus-Icon 75 %. Es gilt für neue Installationen, fehlende VR-Einträge und die Schaltfläche **Standardwerte**. Vorhandene gespeicherte Werte bleiben erhalten. **Maus-Icon → Aus** blendet das Spielsymbol bei Bedarf aus.

## Ultrawide-Auflösungen
Unter **Einstellungen → Grafik → Window size / Fenstergröße** stehen zusätzlich **2560 × 1080** und **3440 × 1440** (Ultrawide, ungefähr 21:9) sowie **3840 × 1080** (32:9) zur Auswahl. Im Vollbildmodus heißt die Auswahl **Fullscreen resolution / Vollbildauflösung**. Im laufenden Spiel ist sie im normalen Einstellungsfenster erreichbar.

Als Einstieg 2560 × 1080 wählen. Die Engine zeichnet tatsächlich in diesem breiten Format; die VR-Bildfläche wird in derselben Pixelgröße neu angelegt. Das Bild wird nicht bloß gestreckt. Die Höhe des räumlichen Fensters folgt seinem Seitenverhältnis; Breite/Abstand bleiben mit dem linken Stick einstellbar. Die höhere Pixelzahl kann mehr Rechenleistung kosten, besonders bei 3440 × 1440.

Die gewählte unterstützte Auflösung wird beim nächsten Start beachtet. Nicht unterstützte alte Startwerte verwenden 1920 × 1200 als Rückfallwert. Auflösungen oberhalb der von Laufzeit und Grafiktreiber gemeldeten Grenzen werden nicht angeboten. Bereits unterstützte 16:10-Formate bleiben verfügbar; zusätzlich gibt es 1920 × 1080 (16:9).

## Größere GUI und Bildfilter
Zusätzliche Renderauflösungen: **3840 × 2160**, **3840 × 2400**, **5120 × 2160** und **5760 × 2400**, sofern vom Gerät unterstützt. Bei 2160 Pixeln Höhe sind bis zu **360 %**, bei 2400 Pixeln bis zu **400 %** GUI-Vergrößerung möglich. Die logische Mindestfläche von 800 × 600 bleibt erhalten, damit Dialoge vollständig bedienbar bleiben. Beim Wechsel auf eine kleinere Auflösung wird eine zu große GUI automatisch begrenzt. Die Maximalgröße steht auch im VR-Schnellmenü.

Bildschärfe verarbeitet das gerenderte Bild mit benachbarten Pixeln. Scharfzeichnen erhöht lokale Kantenkontraste, Weichzeichnen glättet sie. Es erzeugt keine neuen Details in den Originalgrafiken. Die Filterstärke ist begrenzt, um helle/dunkle Säume zu reduzieren. Hohe Auflösungen und Filter können zusätzliche GPU-Leistung kosten; deshalb Auflösung und Schärfe getrennt vergleichen. Transparente Fransen am skalierten Fingerzeiger werden separat abgeschnitten; der originale gezeichnete Rand bleibt Teil der Grafik.

## Weitere VR-Optionen
**Weitere VR-Optionen** beziehungsweise **linke Menütaste → Komfort / Belegung** öffnet Zeigerglättung, Klickstabilisierung, Vibration, Hervorhebung, Zeigerhand, automatische Einzelspieler-Pause, Positionsspeicherung, Listenscrollen, frei belegbare Tasten, Favoriten und Grafikprofile. [Bedienung und Schalter](VR-Komfort.md).
