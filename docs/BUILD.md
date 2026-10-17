# Quest-APK aus den Quellen bauen

Wer eine Oberfläche bevorzugt, verwendet den [plattformübergreifenden Quest APK Builder](GUI-BUILDER.md). Er nutzt denselben Build-Ablauf.

## Voraussetzungen
- Git mit Zugriff auf dieses Repository und die öffentlichen Upstream-Repositories.
- Python 3.9 oder neuer.
- JDK 17; `JAVA_HOME` auf dessen Installationsverzeichnis setzen.
- Android SDK; `ANDROID_HOME` auf dessen Installationsverzeichnis setzen.
- Internet beim ersten Build für Submodule, Gradle und Maven-Abhängigkeiten.

Benötigte SDK-Komponenten, installierbar über Android Studio oder `sdkmanager`:

```sh
sdkmanager "platform-tools" "platforms;android-36" "build-tools;35.0.0" "ndk;27.0.12077973" "cmake;3.22.1"
```

Erforderliche SDK-Lizenzen selbst lesen und über `sdkmanager --licenses` akzeptieren. Android-Tools: [offizielle Downloads](https://developer.android.com/studio). JDK 17 ist z. B. über [Eclipse Temurin](https://adoptium.net/temurin/releases/?version=17) erhältlich. SDK/JDK gehören nicht zum Repository.

Der Gradle Wrapper verwendet Gradle 8.13, das Android-Plugin 8.13.2. NDK 27.0.12077973 und CMake 3.22.1 sind im Projekt festgelegt. Gebaut wird `arm64-v8a` für Quest 3.

## macOS / Linux

```sh
export JAVA_HOME="/Pfad/zum/JDK17"
export ANDROID_HOME="/Pfad/zum/Android-SDK"
python3 scripts/build-quest.py
```

## Windows / PowerShell

```powershell
$env:JAVA_HOME = 'C:\Pfad\zum\JDK17'
$env:ANDROID_HOME = 'C:\Pfad\zum\Android-SDK'
py -3 scripts/build-quest.py
```

Der vollständige Windows-Build wurde am 27.09.2026 unter Windows 10 erfolgreich ausgeführt (CLI, nicht die GUI). Zusätzlich zu den gemeinsamen Voraussetzungen nötig:

- **Python im PATH**, nicht nur über `py`: Der OpenXR-Loader erzeugt Quelltext mit Python.
- **gettext** (`msgfmt`, `msgmerge`) im PATH, z. B. `winget install --id mlocati.GetText`; die Engine kompiliert damit die Übersetzungen.
- Aktuelle Android-Command-line-Tools: `sdkmanager.bat` ist nur noch ein Wrapper, der Paketnamen mit `;` falsch zerlegt. Stattdessen `cmdline-tools\latest\bin\android.exe --no-metrics --sdk=<SDK> sdk install "platforms;android-36" "build-tools;35.0.0" "ndk;27.0.12077973" "cmake;3.22.1"` verwenden und die Ordner danach prüfen (der Exitcode beim Beenden ist unzuverlässig).
- Symlinks sind nicht erforderlich (bzip2 wird nur als Bibliothek gebaut); Entwicklermodus/Adminrechte braucht es nicht.

Einen kurzen Checkout-Pfad verwenden; die tiefen Abhängigkeitsverzeichnisse können Windows-Pfadlängenlimits erreichen.

## Was das Build-Skript macht
1. Festgelegte Submodule initialisieren.
2. ARM64-OpenXR-Loader aus Khronos-Quellen bauen und in den lokalen JNI-Ausgabeordner kopieren.
3. Android-Bibliotheken, Engine und Quest-Treiber mit Gradle bauen.

Es werden keine Originalspieldaten für das Kompilieren benötigt. RTTR-eigene, lizenzierte Ressourcen werden durch den Build erzeugt/kopiert. Build-Ausgaben liegen in ignorierten Verzeichnissen.

Ergebnis: `android/app/build/outputs/apk/debug/xr.rttr-Quest.apk`.

## Auf die Quest übertragen
Vor der Installation das laufende Spiel speichern. Entwicklermodus aktivieren, Quest per USB anschließen und USB-Debugging im Headset bestätigen. [Meta-Anleitung](https://developers.meta.com/horizon/documentation/native/android/mobile-device-setup/)

```sh
python3 scripts/quest.py install
python3 scripts/quest.py import-data "/Pfad/zur/Siedler-II-Gold-Installation"
```

Bei mehreren angeschlossenen Geräten `--serial <ADB-Seriennummer>` vor dem Unterbefehl angeben. `ADB` kann alternativ auf die ausführbare ADB-Datei zeigen. Das Importskript prüft bisher nur die Existenz von `DATA` und `GFX`, nicht die vollständige Ausgabe-Kompatibilität. Vorhandene Originaldaten werden nur mit `--replace-data` überschrieben. Es löscht keine Spielstände; ein Datenwechsel kann dennoch die Nutzbarkeit vorhandener Spiele beeinflussen.

Die Originaldaten liegen anschließend in `/sdcard/RTTR/S2`. Diesen Ordner in der App-Einrichtung auswählen und benötigte Speicherberechtigungen erteilen. Spielstände und Einstellungen liegen getrennt unter `/sdcard/RTTR/.s25rttr`.

Der reguläre Einstieg ist `xr.rttr/org.s25rttr.sdl.GameStartActivity`. `SDLActivity` nicht direkt kaltstarten.

## Signierung und Updates
Der Debug-Build verwendet den lokalen Android-Debug-Schlüssel. Ein anderer Rechner kann einen anderen Schlüssel erzeugen; dann lehnt Android `install -r` über einer bereits installierten App ab. Nicht automatisch deinstallieren: zuerst Spielstände extern sichern und den vorgesehenen Update-/Signierweg klären. Für öffentliche Releases muss eine dauerhafte Signierung eingerichtet werden; private Schlüssel gehören niemals ins Repository.

## Prüfstand
Der ursprüngliche Entwicklungsstand wurde auf Quest 3 gebaut und installiert. Der vollständige Neubau der neuen Repository-Struktur ohne bestehende native Build-Ausgaben war auf macOS erfolgreich; Details und Grenzen siehe [Prüfbericht](VALIDATION.md). Windows, automatischer Import verschiedener Ausgaben und ein grafischer Assistent bleiben offene Prüf-/Entwicklungsaufgaben.
