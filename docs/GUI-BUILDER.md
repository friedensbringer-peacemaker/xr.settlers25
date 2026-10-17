# Quest APK Builder – grafische Oberfläche

Der **Quest APK Builder** ist eine einfache Python-/Tkinter-Oberfläche für Windows, macOS und Linux. Er prüft die Entwicklungswerkzeuge und baut daraus eine ARM64-APK. Originale Siedler-Spieldaten und eine angeschlossene Quest sind für das Kompilieren nicht erforderlich.

Der Builder ersetzt noch nicht den geplanten Installationsassistenten: Entwicklerkonto, APK-Installation und Originaldatenimport sind separate Schritte. Dafür gibt es die [vollständige Quest-Einrichtung](wiki/Ersteinrichtung-Schritt-fuer-Schritt.md).

Vollständige kopierbare Befehlsfolgen: [Windows / PowerShell](setup/Windows-PowerShell.md) und [macOS / Terminal](setup/macOS-Terminal.md). Für Hilfe durch KI-Assistenten gibt es [Start-Prompts](agents/START-PROMPTS.md).

## 1. Einmalige Vorbereitung

1. **Python 3.9 oder neuer mit Tkinter** installieren. Unter Windows und macOS eignen sich die offiziellen [Python-Installer](https://www.python.org/downloads/). Unter Linux gegebenenfalls das Tkinter-Paket der Distribution ergänzen, z. B. `python3-tk` unter Debian/Ubuntu. Homebrew-Python benötigt je nach Installation ein passendes separates Tk-Paket.
2. **Git** über [git-scm.com](https://git-scm.com/downloads) installieren.
3. **JDK 17** installieren, z. B. [Eclipse Temurin 17](https://adoptium.net/temurin/releases/?version=17). Benötigt wird ein JDK, keine reine Java-Laufzeit.
4. **Android SDK** über [Android Studio](https://developer.android.com/studio) installieren. Im SDK Manager folgende Komponenten auswählen: Android SDK Platform 36, Build-Tools 35.0.0, NDK 27.0.12077973 und CMake 3.22.1. Gegebenenfalls „Show Package Details“ aktivieren, um die genaue Version auszuwählen. Die erforderlichen Lizenzen im SDK Manager selbst prüfen und akzeptieren. Platform-Tools sind später für ADB sinnvoll, aber keine Voraussetzung dieser Build-Prüfung.
5. Das Repository mit Git klonen. Ein GitHub-ZIP genügt nicht, weil der Build festgelegte Submodule benötigt:

```sh
git clone https://github.com/friedensbringer-peacemaker/xr.rttr.git
```

Der Builder lädt die Submodule anschließend selbst. Unter Windows einen kurzen Pfad wie `C:\XR\xr.rttr` verwenden. Die Einrichtung der Werkzeuge ist nur einmal nötig; der Builder installiert sie bisher nicht automatisch.

## 2. Oberfläche starten

| System | Start |
| --- | --- |
| macOS | `Quest-Builder.command` im Finder doppelklicken |
| Windows | `Quest-Builder.cmd` doppelklicken |
| Windows / PowerShell | Alternativ `./Quest-Builder.ps1`; falls die lokale Skriptrichtlinie dies verhindert, die CMD-Datei verwenden |
| Linux | Im Projektordner `sh Quest-Builder.command` oder `python3 scripts/quest-builder.py` ausführen |

Alternativ auf allen Systemen direkt `scripts/quest-builder.py` mit Python starten; unter Windows meist `py -3 scripts/quest-builder.py`.

## 3. Werkzeugordner auswählen

Die Oberfläche versucht vorhandene Installationen über Umgebungsvariablen und übliche Installationsordner zu erkennen. Sie zeigt zwei Felder:

- **Android SDK:** den gesamten SDK-Ordner auswählen, nicht dessen `platform-tools`-Unterordner. Beispiele: `~/Library/Android/sdk` auf macOS, `%LOCALAPPDATA%\Android\Sdk` unter Windows oder `~/Android/Sdk` unter Linux. Im Android Studio SDK Manager wird der tatsächliche Pfad angezeigt.
- **Java JDK 17:** den Ordner auswählen, der `bin/java` und `bin/javac` enthält. Auf macOS darf auch das `.jdk`-Paket ausgewählt werden; `Contents/Home` wird automatisch berücksichtigt.

Die Auswahl wird nur lokal unter `.build/builder-settings.json` gespeichert und nicht nach GitHub übertragen. Beim Build wird die gewählte SDK-Position in die ignorierte `android/local.properties` übernommen. Andere globale Einstellungen werden nicht geändert.

## 4. Voraussetzungen prüfen

Auf **„Voraussetzungen prüfen“** klicken. Die Oberfläche kontrolliert Git, JDK-Version, Android-Plattform, Build-Tools, NDK, CMake/Ninja und die vorhandene SDK-Lizenzdatei. Fehlende Komponenten werden im Protokoll namentlich aufgeführt.

Eine erfolgreiche Prüfung bedeutet, dass diese lokalen Voraussetzungen vorhanden sind. Internetzugriff, GitHub-Zugriffsrechte und weitere beim Download geprüfte Lizenzen können anschließend dennoch einen Build verhindern.

## 5. APK bauen

Auf **„APK bauen“** klicken. Das Fenster bleibt während des Builds bedienbar und zeigt Laufzeit sowie das Live-Protokoll. Der Ablauf:

1. Die festgelegten Submodule laden.
2. Den OpenXR-Loader aus Quellen bauen.
3. Die nativen Bibliotheken und die Spiel-Engine bauen.
4. Die Debug-APK verpacken.

Es wird kein erfundener Prozentfortschritt angezeigt; der laufende Balken signalisiert Aktivität. Protokolle bleiben unter `.build/builder-YYYYMMDD-HHMMSS.log` gespeichert.

**„Abbrechen“** beendet den gestarteten Build-Prozess einschließlich seiner Prozessgruppe bzw. unter Windows seines Prozessbaums. Es kann einige Sekunden dauern. Bereits geladene Abhängigkeiten und Build-Zwischendateien bleiben für den nächsten Versuch erhalten. Schließen während eines laufenden Vorgangs fragt nach, ob abgebrochen werden soll.

## 6. Ergebnis verwenden

Nach erfolgreichem Build auf **„APK-Ordner öffnen“** klicken. Die Datei liegt unter:

```text
android/app/build/outputs/apk/debug/xr.rttr-Quest.apk
```

Nach einem Fehler oder Abbruch darf eine dort eventuell vorhandene ältere APK nicht als neues Ergebnis verstanden werden. Der Status in der Oberfläche unterscheidet diese Fälle ausdrücklich.

APK mit ADB oder SideQuest installieren und anschließend die eigenen Originaldaten übertragen: [Ersteinrichtung](wiki/Ersteinrichtung-Schritt-fuer-Schritt.md). Debug-Signierung und mögliche Update-Konflikte sind in der [Build-Anleitung](BUILD.md#signierung-und-updates) erklärt.

## Dauer und geprüfter Stand

Ein vollständiger Build der Repository-Fassung dauerte auf dem Entwicklungs-Mac etwa neun Minuten, zusätzlich zum OpenXR-Loader und vorhandenen Download-Caches. Das ist keine Zusage für andere Rechner: Der erste Download, CPU-Leistung und Internetzugang können den Vorgang deutlich verlängern. Folge-Builds nutzen Zwischendateien.

Auf macOS geprüft: Start mit Tkinter, echte Werkzeugprüfung, Layoutgrenzen der Hauptbedienelemente bei Standardfenstergröße, Fehleranzeige bei vorhandener älterer APK und Abbruch eines Testprozesses. Ein echter APK-Build über den GUI-Prozessablauf wurde erfolgreich ausgeführt; dabei wurden die vorhandenen nativen Build-Caches des zuvor vollständig geprüften Checkouts verwendet. Windows-/Linux-Start, Dateidialoge und Prozessabbruch sind plattformübergreifend implementiert, aber auf diesen Systemen noch nicht praktisch getestet.

## Ohne Oberfläche

```sh
python3 scripts/build-quest.py --check --sdk "/Pfad/zum/SDK" --java "/Pfad/zum/JDK17"
python3 scripts/build-quest.py --sdk "/Pfad/zum/SDK" --java "/Pfad/zum/JDK17"
```

Alternativ weiterhin `ANDROID_HOME` und `JAVA_HOME` setzen. Die Oberfläche und das Kommandozeilenwerkzeug verwenden dieselbe Prüfung und denselben Build-Ablauf.
