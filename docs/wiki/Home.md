# xr.rttr für Meta Quest 3

Stand: 27. September 2026 (2.1.0-preview.4, App-Paket `xr.rttr`). Experimenteller Quest-Port auf Basis von Return to the Roots und dessen Android-Port. Das Spiel erscheint als verstellbares Spielfenster im VR-Raum und wird mit Controllern bedient.

**Ein 100 % Vibe-Coding-Projekt mit Codex, Astra und Claude (Opus).** Die projektspezifische Quest-/VR-Entwicklung entsteht vollständig im Vibe-Coding-Verfahren. Grundlage sind die bestehenden Open-Source-Projekte Return to the Roots und dessen Android-Port; deren Urheberschaft und Lizenzhinweise bleiben erhalten.

## Bezugsquellen
Die benötigten Originaldaten erhältst du aus deiner eigenen **Siedler-II-Gold-Installation**, zum Beispiel über [GOG](https://www.gog.com/en/game/the_settlers_2_gold_edition). Die freie Engine findest du auf der [RTTR-Website](https://www.siedler25.org/index.php). Engine und Originalspieldaten sind getrennte Bestandteile. Siehe auch [Startseite](../../README.md).

## APK-Builder
Die Python-/Tkinter-Oberfläche kann Voraussetzungen prüfen und die APK bauen. [Anleitung für Windows, macOS und Linux](../GUI-BUILDER.md). Der vollständige Installations-/Datenimport-Assistent bleibt ein separates geplantes Vorhaben.

## Konfiguration mit Befehlen oder Agenten
[Windows / PowerShell](../setup/Windows-PowerShell.md) · [macOS / Terminal](../setup/macOS-Terminal.md) · [Agenten-Start-Prompts](../agents/START-PROMPTS.md)

## Seiten
- [Ersteinrichtung Schritt für Schritt](Ersteinrichtung-Schritt-fuer-Schritt.md)
- [Quest-Bedienung](Quest-Bedienung.md)
- [Installation und Einrichtungsassistent](Installation-und-Einrichtungsassistent.md)
- [Spieldaten und Kompatibilität](Spieldaten-und-Kompatibilitaet.md)

## Aktueller Stand
- preview.4: Jeder App-Start zeigt die Leinwand zentriert vor der aktuellen Blickrichtung. Optional „Trigger halten: Karte greifen“ (Komfort / Belegung → Controller → Navigation). APK-Build jetzt auch unter Windows erfolgreich.
- OpenXR-Darstellung mit separatem Quad-Layer für das Spielbild; die deutlich bessere Schärfe wurde im Headset vom Nutzer bestätigt.
- Controllerstrahl und Mausbedienung; Größe und Entfernung des Panels einstellbar.
- Bildschirmtastatur im VR-Speicherfenster implementiert. Vollständiger Eingabe-/Speichertest im Headset noch offen.
- Geschwindigkeitszyklus auf linkem Y und erweiterter Panel-Nahbereich implementiert, gebaut und auf der Quest installiert. Bedienung dieser letzten Änderungen noch zu bestätigen.
- Ein Windows-/macOS-Einrichtungsassistent ist geplant, noch nicht verfügbar.
- Angepasster Android-/Engine-Quellcode, festgelegte Abhängigkeiten und Build-/ADB-Hilfsskripte liegen im Repository. Siehe [Build-Anleitung](../BUILD.md).

## Offene Verbesserungen
- Panelposition und -größe dauerhaft speichern; zur aktuellen Blickrichtung zentrieren.
- Größere Trefferflächen und klarere Rückmeldung bei Controllerklicks.
- Bildschirmtastatur für weitere Textfelder.
- Schwarzen Bereich beim Chat reproduzieren und beheben.
- Längere Spielsitzungen, Updates und Spielstandsicherung systematisch prüfen.

## Projektbasis
- [Return to the Roots](https://github.com/Return-To-The-Roots/s25client)
- [Android-Port](https://github.com/Farmer-Markus/s25rttr-android)

Originalspieldaten werden vom Nutzer aus seiner eigenen Installation bereitgestellt; sie sind nicht Bestandteil der geplanten Verteilung.

## VR-Anpassungen
[Controllerachsen, Helligkeit, Kontrast, Fensterwölbung und Zeiger konfigurieren](VR-Optionen.md).

- [Diorama und Ansichten](Diorama-und-Ansichten.md): umschaltbare Leinwand/Tischansicht und nächste Schritte zum echten 3D-Gelände.

## Projektpflege

- [Updatelog](../../CHANGELOG.md)
- [Backlog](../../BACKLOG.md)
- [Zentrale Versionsdatei](../../version.properties)
