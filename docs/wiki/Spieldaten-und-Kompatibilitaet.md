# Spieldaten und Kompatibilität

## Originaldaten und Engine unterscheiden
Return to the Roots (oft Siedler 2.5 genannt) ist die Engine. Eine GOG-Ausgabe bezeichnet eine Bezugsquelle/Ausgabe des Originalspiels. Das sind nicht automatisch unterschiedliche Spielmodi oder Quest-APKs.

Die README der verwendeten Engine nennt **The Settlers II Gold Edition** als Grundlage und beschreibt den Import der Ordner `DATA` und `GFX`. Siehe [Engine-Installationsanleitung](https://github.com/Return-To-The-Roots/s25client#how-to-install).

## Offizielle Bezugsquellen
- [Siedler II Gold Edition bei GOG](https://www.gog.com/en/game/the_settlers_2_gold_edition): Kauf und Download über das eigene Konto.
- [Return to the Roots](https://www.siedler25.org/index.php): freie Engine und Informationen.
- Bereits vorhandene eigene Gold-Installation: deren `DATA`-/`GFX`-Ordner verwenden.

Diese Bezugsquellen ersetzen nicht den noch offenen Importtest der jeweiligen Ausgabe.

## Prüfstand
| Datenquelle | Stand für den geplanten Assistenten |
| --- | --- |
| Originale Siedler-II-Gold-Daten | Grundlage laut Engine; automatischer Import noch zu implementieren |
| GOG-Ausgabe von Siedler II Gold | Gewünschte Datenquelle; Erkennung und Import noch separat zu testen |
| Vorhandene RTTR-/2.5-Installation | Geeignete Originaldaten darin suchen; nicht den ganzen Programmordner ungeprüft übernehmen |
| Weitere Editionen, Sprachversionen und Neuauflagen | Keine pauschale Kompatibilitätszusage; einzeln prüfen |

Der vorhandene Entwicklungsaufbau läuft mit lokal bereitgestellten S2-Daten. Daraus folgt keine Abnahme sämtlicher Ausgaben.

## Anforderungen an die Erkennung
- Ordnerstruktur und benötigte Dateien prüfen, nicht nur den Ordnernamen.
- Passende Originaldaten getrennt von Engine, Einstellungen und Spielständen übernehmen.
- Fehlende Dateien und nicht unterstützte Daten verständlich benennen.
- Unterschiede zwischen Sprach-/Spielausgaben mit reproduzierbaren Import- und Starttests dokumentieren.
- Originaldaten nicht im Projekt, in Releases oder in der Wiki verteilen.

Der erste Umsetzungsschritt ist eine Prüfung der tatsächlichen Anforderungen der verwendeten Engine. Danach entsteht eine getestete Kompatibilitätsliste für den Assistenten.

Weitere Hintergrundinformationen: [offizielle RTTR-FAQ](https://www.siedler25.org/index.php?com=static&mod=1) und [GitHub-Organisation](https://github.com/Return-To-The-Roots). Die FAQ enthält auch ältere Verweise; für den konkreten Build sind die im Repository festgelegten Quellen maßgeblich.
