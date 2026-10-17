# Windows: Befehle von der Vorbereitung bis zum ersten Start

Für Windows 10/11 mit PowerShell. Die Blöcke **nacheinander** ausführen; nach einem Fehler anhalten. Eingaben im Browser und im Headset lassen sich nicht durch Terminalbefehle ersetzen. Der Build selbst wurde am 27.09.2026 unter Windows 10 erfolgreich ausgeführt; die komplette Abfolge auf einem frisch eingerichteten Rechner ist noch nicht durchgespielt. Hinweise zu gettext und den neuen Android-Command-line-Tools stehen in der [Build-Anleitung](../BUILD.md#windows--powershell).

Wer nur die fertige APK installieren möchte, braucht keine Build-Umgebung: dafür die [Einrichtungsanleitung](../wiki/Ersteinrichtung-Schritt-fuer-Schritt.md) verwenden.

## 1. Werkzeuge installieren

PowerShell öffnen und WinGet prüfen:

```powershell
winget --version
```

Fehlt WinGet, den [Microsoft App Installer / WinGet](https://learn.microsoft.com/windows/package-manager/winget/) einrichten. Dann diese Pakete einzeln installieren und die angezeigten Bedingungen selbst prüfen:

```powershell
winget install --id Git.Git --exact --source winget
winget install --id GitHub.cli --exact --source winget
winget install --id Python.Python.3.13 --exact --source winget
winget install --id EclipseAdoptium.Temurin.17.JDK --exact --source winget
winget install --id mlocati.GetText --exact --source winget
```

Paketkennungen stammen aus dem [Microsoft-Paketverzeichnis](https://github.com/microsoft/winget-pkgs). Nach der Installation **PowerShell schließen und neu öffnen**, damit PATH aktualisiert ist.

```powershell
$ErrorActionPreference = 'Stop'
git --version
gh --version
py -3.13 --version
py -3.13 -c "import tkinter; print('Tkinter:', tkinter.TkVersion)"
```

## 2. Java und SDK-Ordner setzen

```powershell
$QuestJdk = Get-ChildItem -LiteralPath "$env:ProgramFiles\Eclipse Adoptium" -Directory -Filter 'jdk-17*' | Sort-Object Name -Descending | Select-Object -First 1
if (-not $QuestJdk) { throw 'JDK 17 fehlt. Installation prüfen.' }
$env:JAVA_HOME = $QuestJdk.FullName
$env:ANDROID_HOME = Join-Path $env:LOCALAPPDATA 'Android\Sdk'
$env:ANDROID_SDK_ROOT = $env:ANDROID_HOME
$env:PATH = "$env:JAVA_HOME\bin;$env:ANDROID_HOME\platform-tools;$env:PATH"
& "$env:JAVA_HOME\bin\java.exe" -version
```

Diese Variablen gelten für dieses Fenster. In einer neuen Sitzung diesen Block erneut ausführen oder die Ordner direkt im Builder auswählen.

## 3. Android Command-line Tools und Komponenten

Ist Android Studio mit den passenden Werkzeugen bereits eingerichtet, kann der Download entfallen. Sonst:

```powershell
Start-Process 'https://developer.android.com/studio#command-tools'
```

Auf der offiziellen Seite **Command line tools only → Windows** als ZIP laden und die Bedingungen prüfen. Dann den tatsächlichen vollständigen ZIP-Pfad eingeben:

```powershell
$QuestSdkTools = Join-Path $env:ANDROID_HOME 'cmdline-tools\latest'
if (-not (Test-Path (Join-Path $QuestSdkTools 'bin\sdkmanager.bat'))) {
    $QuestZip = (Read-Host 'Vollständiger Pfad zur heruntergeladenen Windows-Command-line-Tools-ZIP').Trim('"')
    $QuestZip = (Resolve-Path -LiteralPath $QuestZip).Path
    $QuestExtract = Join-Path $env:TEMP ('QuestSdk-' + [guid]::NewGuid().ToString())
    Expand-Archive -LiteralPath $QuestZip -DestinationPath $QuestExtract
    if (-not (Test-Path (Join-Path $QuestExtract 'cmdline-tools\bin\sdkmanager.bat'))) { throw 'Falsche ZIP-Struktur.' }
    if (Test-Path $QuestSdkTools) { throw 'Zielordner existiert bereits, ist aber unvollständig. Zuerst prüfen.' }
    New-Item -ItemType Directory -Force -Path (Split-Path $QuestSdkTools) | Out-Null
    Move-Item -LiteralPath (Join-Path $QuestExtract 'cmdline-tools') -Destination $QuestSdkTools
}
$QuestSdkManager = Join-Path $QuestSdkTools 'bin\sdkmanager.bat'
& $QuestSdkManager "--sdk_root=$env:ANDROID_HOME" --licenses
if ($LASTEXITCODE -ne 0) { throw 'SDK-Lizenzschritt fehlgeschlagen.' }
& $QuestSdkManager "--sdk_root=$env:ANDROID_HOME" 'platform-tools' 'platforms;android-36' 'build-tools;35.0.0' 'ndk;27.0.12077973' 'cmake;3.22.1'
if ($LASTEXITCODE -ne 0) { throw 'SDK-Komponenten konnten nicht installiert werden.' }
```

Lizenzfragen lesen und selbst beantworten. [Offizielle sdkmanager-Anleitung](https://developer.android.com/tools/sdkmanager). Falls aktuelle Command-line Tools eine andere Java-Version verlangen, die SDK-Komponenten über Android Studio installieren; der Projektbuild verwendet weiterhin JDK 17.

## 4. Privates Repository beziehen

```powershell
gh auth login --hostname github.com --git-protocol https --web
```

Die Browseranmeldung abschließen. Keine Passwörter oder Tokens in Befehle oder einen KI-Chat kopieren.

```powershell
$QuestWorkspace = Join-Path $env:USERPROFILE 'XR'
New-Item -ItemType Directory -Force -Path $QuestWorkspace | Out-Null
Set-Location $QuestWorkspace
gh repo clone friedensbringer-peacemaker/xr.rttr
if ($LASTEXITCODE -ne 0) { throw 'Klonen fehlgeschlagen; Zugriff oder vorhandenen Zielordner prüfen.' }
Set-Location (Join-Path $QuestWorkspace 'xr.rttr')
git status --short
```

Ein bereits vorhandener Checkout wird nicht überschrieben: dann direkt dorthin wechseln. Bei Pfadlängenproblemen einen kürzeren, beschreibbaren Arbeitsordner wählen. Ein Source-ZIP ersetzt den Git-Checkout nicht.

## 5. Prüfen und bauen

```powershell
py -3.13 scripts/build-quest.py --check --sdk "$env:ANDROID_HOME" --java "$env:JAVA_HOME"
if ($LASTEXITCODE -ne 0) { throw 'Voraussetzungen zuerst korrigieren.' }
```

**Mit Oberfläche:**

```powershell
py -3.13 scripts/quest-builder.py
```

Dort „Voraussetzungen prüfen“ und „APK bauen“ wählen. Oder **stattdessen im Terminal**:

```powershell
py -3.13 scripts/build-quest.py --sdk "$env:ANDROID_HOME" --java "$env:JAVA_HOME"
if ($LASTEXITCODE -ne 0) { throw 'Build fehlgeschlagen; keine alte APK als neues Ergebnis verwenden.' }
Get-Item '.\android\app\build\outputs\apk\debug\xr.rttr-Quest.apk'
```

Nicht beide Builds gleichzeitig starten.

## 6. Quest vorbereiten und verbinden

Meta-Konto/Team registrieren und verifizieren, Entwicklermodus aktivieren, Windows-ADB-Treiber installieren und USB-Debugging im Headset bestätigen: [Schritt-für-Schritt mit offiziellen Links und Videos](../wiki/Ersteinrichtung-Schritt-fuer-Schritt.md). Dann:

```powershell
& "$env:ANDROID_HOME\platform-tools\adb.exe" devices
```

Nur fortfahren, wenn bei der richtigen Quest `device` steht. Bei mehreren Geräten die gewünschte Seriennummer mit `--serial` an die folgenden Python-Befehle übergeben, z. B. `scripts/quest.py --serial SERIENNUMMER install`.

## 7. Sichern, installieren und eigene Daten übertragen

Bei einer bereits benutzten Installation zuerst im Spiel speichern und sichern:

```powershell
$QuestBackup = Join-Path $env:USERPROFILE ('RTTR-Backup-' + (Get-Date -Format 'yyyyMMdd-HHmmss'))
& "$env:ANDROID_HOME\platform-tools\adb.exe" pull '/sdcard/RTTR/.s25rttr' "$QuestBackup"
```

Bei einer Erstinstallation existiert der Sicherungsordner auf der Quest noch nicht. Bei einer bestehenden Installation ein fehlgeschlagenes Backup zuerst klären. Danach:

```powershell
py -3.13 scripts/quest.py install
if ($LASTEXITCODE -ne 0) { throw 'Installation fehlgeschlagen. Bei Signaturkonflikten nicht einfach deinstallieren.' }
$QuestOriginals = (Read-Host 'Ordner deiner eigenen Siedler-II-Gold-Installation mit DATA und GFX').Trim('"')
py -3.13 scripts/quest.py import-data "$QuestOriginals"
if ($LASTEXITCODE -ne 0) { throw 'Datenimport fehlgeschlagen oder vorhandene Daten erkannt. Meldung prüfen.' }
& "$env:ANDROID_HOME\platform-tools\adb.exe" shell am start -n xr.rttr/org.s25rttr.sdl.GameStartActivity
```

Vorhandene Originaldaten werden vom Skript standardmäßig abgelehnt. `--replace-data` nur für einen bewusst gewählten Datenwechsel verwenden. In der App `/sdcard/RTTR/S2` als Originaldatenordner wählen und benötigte Speicherberechtigungen erteilen. Anschließend Menü, Controller, Speichern und Laden testen.

## 8. Später erneut starten

Im Repository `Quest-Builder.cmd` doppelklicken. Die GUI merkt sich die Werkzeugordner. Für erneute Terminalnutzung die Variablen aus Schritt 2 setzen. [Builder-Hilfe](../GUI-BUILDER.md)
