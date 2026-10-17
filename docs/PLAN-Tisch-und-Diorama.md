# Plan: Echter Tisch (VR-022) und Diorama-Gelände (VR-023)

Stand 27.09.2026, Ausgangsbasis `v2.1.0-preview.8`. Beide Themen sind größere Umbauten und werden in einzeln abnehmbaren Stufen umgesetzt. Jede Stufe endet mit einer eigenen Vorabversion und einer Headset-Prüfung. Die Leinwand bleibt immer unverändert als sicherer Rückweg.

## VR-022 – Spielbrett auf den echten Tisch legen

**Ziel:** Nach einem Raumscan der Quest liegt das Spielbrett auf der Oberfläche des realen Schreibtischs: in passender Höhe und Ausrichtung, im Passthrough sichtbar.

**Technik:** Die Quest stellt erkannte Möbel als räumliche Objekte mit semantischer Bezeichnung bereit (z. B. `TABLE`, `DESK`) und liefert jeweils eine begrenzte 2D-Fläche und eine Pose. Benötigte Erweiterungen, im eingebundenen OpenXR-SDK vorhanden: `XR_FB_spatial_entity`, `XR_FB_spatial_entity_query`, `XR_FB_spatial_entity_storage`, `XR_FB_scene`, optional `XR_FB_scene_capture` zum Starten des Raumscans. Android-Berechtigung: `com.oculus.permission.USE_SCENE`; die Freigabe fragt die Quest einmalig im Headset ab.

| Stufe | Inhalt | Abnahme im Headset |
| --- | --- | --- |
| 1 | Erweiterungen und Berechtigung prüfen, Raumobjekte abfragen, gefundene Tische mit Größe/Höhe ins Log (`RTTR-XR`) schreiben. Keine sichtbare Änderung. | Log zeigt deinen Schreibtisch mit plausibler Höhe und Fläche. |
| 2 | Neue Tischvariante „Auf echtem Tisch“: Brett flach (0°) auf die Tischfläche legen, Mittelpunkt und Ausrichtung aus der Pose, Breite an die Tischfläche angepasst (mit Obergrenze). Strahltreffer nutzen dieselbe Pose. | Brett liegt im Passthrough auf dem Tisch, Klicks treffen. |
| 3 | Komfort: Auswahl bei mehreren Tischen, Neigung 0–30° als Leselage, leichte Anhebung gegen Flimmern, Rückfall auf den simulierten Tisch ohne Scan. Knopf „Raum scannen“ startet den Quest-Raumscan. | Neustart, anderer Raum, kein Scan. |

**Risiken:** Ohne Raumscan gibt es keine Daten → Rückfall auf den Simulator. Die Tischerkennung ist nur so genau wie der Scan. Die Neigung 0° liest sich schlechter als 30–45° → deshalb in Stufe 3 wählbar.

## VR-023 – Gelände als echtes Diorama

**Ziel:** Berge und Täler haben echte räumliche Tiefe, statt nur ins flache Bild gemalt zu sein.

**Ausgangslage:** Das Spiel rendert die Karte als 2D-Bild auf ein Quad. Die Höhen stecken in `MapNode::altitude` und werden heute nur als Verschiebung und Schattierung in 2D gezeichnet. Eine Verformung des fertigen Bildes würde auch Gebäude, Figuren und Menüs verzerren und scheidet deshalb aus.

| Stufe | Inhalt | Abnahme im Headset |
| --- | --- | --- |
| 1 | **Höhenrelief als Unterlage:** Aus den Höhen des sichtbaren Kartenausschnitts ein grobes Dreiecksnetz erzeugen und es im Projektions-Layer unter das Brett legen, eingefärbt nach Geländetyp. Das Spielbild bleibt vorerst flach darüber. Prototyp, abschaltbar (Beta). | Relief folgt Scrollen und Zoom, passt zum Bild, kostet wenig Leistung. |
| 2 | **Geländetextur aufs Netz:** Den Gelände-Pass des `TerrainRenderer` in eine eigene Textur ausgeben und auf das Netz legen. Wasser, Wege und Grenzen liegen damit auf dem echten Relief. | Kein Versatz zwischen Relief und Wegen, Fog of War korrekt. |
| 3 | **Objekte auf dem Relief:** Gebäude, Bäume und Figuren als Billboards in richtiger Höhe und Reihenfolge; Klicks per Strahl-Dreieck-Schnitt auf den Kartenknoten abbilden. | Bauen, Straßen ziehen, Auswählen funktionieren wie im 2D-Modus. |

**Risiken und Aufwand:** Stufe 1 ist überschaubar und zeigt schnell, ob der Effekt trägt. Die Stufen 2–3 greifen tief in die Engine ein (Render-Pässe, Trefferlogik) und sind der eigentliche Großumbau. Die Leistung muss auf großen Karten gemessen werden (VR-013). Das bestehende 3D-HQ zeigt, dass Stereo-Tiefe und separater Tiefenpuffer im Treiber bereits funktionieren.

## Reihenfolge

1. VR-022 Stufe 1+2: schneller sichtbarer Nutzen, überschaubarer Eingriff nur im OpenXR-Treiber.
2. VR-023 Stufe 1: zeigt früh, ob die Diorama-Wirkung überzeugt, bevor die Engine umgebaut wird.
3. Danach je nach Ergebnis VR-022 Stufe 3 bzw. VR-023 Stufen 2–3.

Quellen: [Meta – OpenXR Scene Overview](https://developers.meta.com/horizon/documentation/native/android/openxr-scene-overview/) · [Meta – Scene API Reference](https://developers.meta.com/horizon/documentation/native/android/mobile-scene-api-ref/) · [Khronos-Registry im Repo](../third_party/openxr-sdk-source/specification/registry/xr.xml)
