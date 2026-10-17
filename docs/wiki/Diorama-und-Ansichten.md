# Diorama und Ansichten

## Jetzt verfügbar

Die Leinwand und ein neigbares Spielbrett zeigen dieselbe laufende Partie. Einstellung und Wechsel siehe [VR-Komfort](VR-Komfort.md#ansicht-leinwand-und-spielbrett). Die Tischansicht ist bewusst als 2D-Spielbrett bezeichnet. Ihre Implementierung verändert weder Spielsimulation noch Originalgrafiken.

Die Neigung dreht um die Bildmitte. Eine gemeinsame Pose liefert Compositor-Orientierung, Position, Strahlrücktransformation und Zeigerposition. Dadurch bleibt die Zuordnung von Controllerziel zu Spielpixel auch bei gedrehter und geneigter Ansicht gleich. Die gebogene Leinwand bleibt als Alternative erhalten.

## Nächste Schritte für ein echtes Diorama

1. **Gelände separat ausgeben:** RTTR hält Kartenhöhen in `MapNode::altitude`. `GameWorldView::Draw` ruft den `TerrainRenderer` vor Objekten, Figuren und GUI auf. Dort lässt sich ein eigener Gelände-Pass ansetzen. Eine beliebige Verformung des fertigen Spielbilds würde auch Gebäude und Menüs verbiegen und ist deshalb kein Ersatz.
2. **Ansichten von derselben Simulation speisen:** Ein gemeinsamer sichtbarer Kartenausschnitt muss Gelände-Knoten, Texturkoordinaten und Höhen bereitstellen. Karten-Wrapping, Zoom und Sichtbarkeit/Fog of War gehören in diese Schnittstelle; unbekannte Landschaft darf nicht zusätzlich sichtbar werden.
3. **Gelände im OpenXR-Projektionslayer zeichnen:** Dreiecksnetz mit Tiefenpuffer und getrennten Augenansichten. Aktuell zeichnet dieser Layer nur den Strahl; das Spielbild bleibt ein Quad-/Cylinder-Compositor-Layer. Keine pauschale Migration auf eine andere Engine notwendig.
4. **Gebäude und Figuren getrennt darstellen:** Zunächst geeignete Sprite-Billboards mit korrekter Höhe, Sortierung und Verdeckung. Spätere 3D-Modelle sind optional; Simulation, Warenwege und Animationen müssen erhalten bleiben.
5. **Bedienung trennen:** Menüs/Tastatur als gut lesbare Fläche, während Kartenklicks per Strahl-Dreieck-Treffer wieder auf den richtigen Kartenpunkt abgebildet werden. Die heutigen Pixelkoordinaten genügen für ein echtes Gelände nicht.
6. **Komfort und Leistung prüfen:** Höhenverstärkung, Diorama-Größe und Blickwinkel im VR-Menü; Vergleich auf der Quest mit großen Karten und vielen Figuren. Passthrough erst als eigener optionaler Schritt.

Diese Punkte beschreiben geplante Arbeit, keine bereits implementierten Funktionen. Erste Abnahme der Tischansicht: Neigung/Höhe, Randbuttons, Straßenbau-Ziehen, Tastatur, Rückwechsel und Neustart mit gespeicherten Einstellungen im Headset testen.

## Eigenes 3D-Hauptquartier (Beta) erzeugen

Das Beta-Hauptquartier in der Tischansicht lädt die private Datei `/sdcard/RTTR/vr-assets/headquarters.xrhq`; ohne sie bleibt der Original-Sprite. Die Datei wird weder ins Repository noch in die APK aufgenommen.

1. Modell erzeugen, z. B. mit Hyper3D (Rodin), und als **schattierten GLB-Export** speichern: ein Mesh, keine Transformation, Emissionstextur mit gebackener Schattierung.
2. Konvertieren (Python mit numpy, Pillow und `meshoptimizer==0.2.30a0`, am besten in einer eigenen venv):
   `python scripts/convert-hq-glb.py QUELLE.glb AUSGABEORDNER`
   Gleiche Positions-/UV-Punkte werden verschmolzen und UV-bewusst auf rund 50.000 Dreiecke reduziert; die Textur wird auf höchstens 2048 × 2048 verkleinert. Ergebnis: `headquarters.xrhq`, dazu `texture.png`, `preview.npz` und `report.json` mit den Kennzahlen.
3. Auf die Quest kopieren: `adb push AUSGABEORDNER/headquarters.xrhq /sdcard/RTTR/vr-assets/` und das Spiel neu starten.

Die bisherige Fassung (26.09.2026) wurde von 1.000.000 auf 49.999 Dreiecke reduziert (37.149 Punkte, 2048×2048-Textur, 18.120.208 Bytes). Eine Sicherung liegt außerhalb des Repositorys unter `<XR-Ordner>\Meta Quest 3\Assets\xr.rttr\`.