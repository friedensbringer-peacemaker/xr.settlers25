# macOS: Befehle von der Vorbereitung bis zum ersten Start

Für Terminal mit zsh oder bash, auf Apple Silicon oder Intel. Die Blöcke **nacheinander** ausführen, bei Fehlern anhalten. Browser- und Headset-Bestätigungen bleiben manuell. Der Projektbuild und die GUI wurden auf dem Entwicklungs-Mac geprüft; diese komplette Homebrew-Neuinstallation auf einem frischen Mac wurde noch nicht durchgespielt.

Wer nur eine fertige APK installieren möchte, benötigt keinen Compiler: [Quest-Ersteinrichtung](../wiki/Ersteinrichtung-Schritt-fuer-Schritt.md).

## 1. Homebrew und Werkzeuge

Prüfen, ob Homebrew vorhanden ist:

```sh
command -v brew
```

Falls nicht vorhanden: [Homebrew-Installationsanleitung](https://brew.sh/) lesen und den dort angebotenen Installer ausführen:

```sh
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

Das installiert einen Paketmanager auf dem Mac. Die angezeigten Schritte und eventuelle Xcode-Command-Line-Tools-Installation abschließen. Dann Homebrew für dieses Terminal laden:

```sh
if [ -x /opt/homebrew/bin/brew ]; then
    eval "$(/opt/homebrew/bin/brew shellenv)"
elif [ -x /usr/local/bin/brew ]; then
    eval "$(/usr/local/bin/brew shellenv)"
fi
brew --version
brew install git gh python@3.13 python-tk@3.13 openjdk@17
brew install --cask android-commandlinetools
```

Bei bereits anders installierten Werkzeugen nicht unnötig doppelt installieren, sondern die vorhandenen Pfade verwenden. Offizielle Paketseiten: [Tkinter](https://formulae.brew.sh/formula/python-tk@3.13), [JDK 17](https://formulae.brew.sh/formula/openjdk@17), [Android Command-line Tools](https://formulae.brew.sh/cask/android-commandlinetools).

## 2. Pfade setzen und SDK installieren

```sh
QUEST_PYTHON="$(brew --prefix python@3.13)/bin/python3.13"
export JAVA_HOME="$(brew --prefix openjdk@17)/libexec/openjdk.jdk/Contents/Home"
export ANDROID_HOME="$HOME/Library/Android/sdk"
export ANDROID_SDK_ROOT="$ANDROID_HOME"
export PATH="$JAVA_HOME/bin:$ANDROID_HOME/platform-tools:$PATH"
"$QUEST_PYTHON" --version
"$QUEST_PYTHON" -c 'import tkinter; print("Tkinter:", tkinter.TkVersion)'
"$JAVA_HOME/bin/java" -version
mkdir -p "$ANDROID_HOME"
sdkmanager --sdk_root="$ANDROID_HOME" --licenses
sdkmanager --sdk_root="$ANDROID_HOME" 'platform-tools' 'platforms;android-36' 'build-tools;35.0.0' 'ndk;27.0.12077973' 'cmake;3.22.1'
```

Lizenzfragen lesen und selbst beantworten; nicht automatisch mit `yes` durchbestätigen. [sdkmanager-Dokumentation](https://developer.android.com/tools/sdkmanager). Falls die aktuell heruntergeladenen Command-line Tools eine andere Java-Version verlangen, SDK-Komponenten über Android Studio installieren; für das Projekt weiterhin JDK 17 auswählen.

Variablen gelten nur in diesem Terminal. Für eine neue Sitzung diesen Pfadblock erneut ausführen oder die Ordner im Builder auswählen. Kein Windows-ADB-Treiber auf dem Mac nötig.

## 3. Privates Repository beziehen

```sh
gh auth login --hostname github.com --git-protocol https --web
```

Browseranmeldung abschließen; das Konto benötigt Repository-Zugriff. Keine Tokens oder Passwörter in Chats einfügen.

```sh
mkdir -p "$HOME/AI"
cd "$HOME/AI"
gh repo clone friedensbringer-peacemaker/xr.rttr
cd "$HOME/AI/xr.rttr"
git status --short
```

Wenn der Zielordner bereits existiert, nicht löschen oder darüberkopieren: direkt den richtigen vorhandenen Checkout verwenden. Ein GitHub-ZIP reicht für die Submodule nicht.

## 4. Prüfen und bauen

```sh
"$QUEST_PYTHON" scripts/build-quest.py --check --sdk "$ANDROID_HOME" --java "$JAVA_HOME"
```

Nur nach erfolgreicher Prüfung fortfahren. **Mit Oberfläche:**

```sh
"$QUEST_PYTHON" scripts/quest-builder.py
```

Im Fenster „Voraussetzungen prüfen“ und „APK bauen“ wählen. Oder **stattdessen im Terminal**:

```sh
"$QUEST_PYTHON" scripts/build-quest.py --sdk "$ANDROID_HOME" --java "$JAVA_HOME"
ls -lh android/app/build/outputs/apk/debug/xr.rttr-Quest.apk
```

Nicht beide Builds gleichzeitig starten. Nach einem fehlgeschlagenen Build ist eine ältere vorhandene APK kein neues Ergebnis.

## 5. Quest vorbereiten

Meta-Konto/Team und Verifizierung, Entwicklermodus und USB-Debugging im Headset anhand der [Ersteinrichtung](../wiki/Ersteinrichtung-Schritt-fuer-Schritt.md) abschließen. Dann:

```sh
"$ANDROID_HOME/platform-tools/adb" devices
```

Bei der Quest muss `device` stehen. `unauthorized`: Freigabe im Headset bestätigen. Bei mehreren Geräten `--serial SERIENNUMMER` vor `install` bzw. `import-data` an die Python-Befehle übergeben.

## 6. Sichern, installieren und Originaldaten importieren

Bei einer bereits benutzten Installation zuerst im Spiel speichern und die Benutzerdaten in einen neuen Ordner sichern:

```sh
QUEST_BACKUP="$HOME/RTTR-Backup-$(date +%Y%m%d-%H%M%S)"
"$ANDROID_HOME/platform-tools/adb" pull /sdcard/RTTR/.s25rttr "$QUEST_BACKUP"
```

Bei der Erstinstallation gibt es auf der Quest noch keinen solchen Datenordner. Bei einer bestehenden Installation ein fehlgeschlagenes Backup zuerst klären. Danach:

```sh
"$QUEST_PYTHON" scripts/quest.py install
```

Nur nach erfolgreicher Installation fortfahren. Bei Signaturkonflikten nicht ungesichert deinstallieren.

Den folgenden Beispielpfad durch den Ordner der **eigenen Gold-Installation** mit `DATA` und `GFX` ersetzen:

```sh
QUEST_ORIGINALS="$HOME/Spiele/Siedler2Gold"
ls "$QUEST_ORIGINALS/DATA" "$QUEST_ORIGINALS/GFX"
"$QUEST_PYTHON" scripts/quest.py import-data "$QUEST_ORIGINALS"
"$ANDROID_HOME/platform-tools/adb" shell am start -n xr.rttr/org.s25rttr.sdl.GameStartActivity
```

Originaldaten beispielsweise aus der eigenen [GOG-Gold-Edition](https://www.gog.com/en/game/the_settlers_2_gold_edition) bereitstellen. Die Extraktion beliebiger Windows-GOG-Installer auf macOS ist noch nicht automatisiert. Ein schon vorhandener passender Spieleordner wird benötigt; den GOG-Installer selbst nicht als Datenordner auswählen.

Vorhandene Daten lehnt das Skript ohne `--replace-data` ab. Den Schalter nur für einen bewussten Datenwechsel verwenden. In der App `/sdcard/RTTR/S2` wählen, Speicherberechtigungen erteilen und Menü, Controller, Speichern und Laden testen.

## 7. Später erneut öffnen

`Quest-Builder.command` im Finder doppelklicken. Falls mehrere Python-Installationen vorhanden sind und dabei Tkinter fehlt, den Start aus Schritt 4 mit dem expliziten Homebrew-Python verwenden. [Builder-Hilfe](../GUI-BUILDER.md)
