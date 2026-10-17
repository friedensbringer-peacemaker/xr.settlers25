# Ersteinrichtung: vom neuen Headset bis zum ersten Spiel

Stand: 26. September 2026. Für Meta Quest 3, Windows und macOS. Die Kontoeinrichtung und Menübezeichnungen können sich ändern; bei Abweichungen die verlinkten aktuellen Herstelleranleitungen verwenden.

## 1. Was du brauchst

| Bestandteil | Wofür? |
| --- | --- |
| Eingerichtete Quest 3 und beide Controller | Spiel ausführen und bedienen |
| Windows-PC oder Mac mit Internet | APK und eigene Spieldaten übertragen |
| USB-Kabel mit Datenübertragung | Verbindung zur Quest; ein reines Ladekabel reicht nicht |
| Smartphone mit Meta Horizon App | Headset verwalten und Entwicklermodus aktivieren |
| Dasselbe Meta-Konto auf Headset, Smartphone und Entwicklerportal | Freigaben dem richtigen Gerät zuordnen |
| Eigene Siedler-II-Gold-Installation | Originaldaten `DATA` und `GFX` |
| Quest-APK dieses Projekts | Die angepasste App, nicht die Desktop-Version von RTTR |
| ADB oder SideQuest Desktop | APK installieren; Spieldaten gesondert übertragen |

**ADB und SideQuest sind Alternativen für die APK-Installation. Du musst nicht beide installieren.** Die folgenden ADB-Befehle decken zusätzlich die Datenübertragung ab. JDK, NDK und Android Studio brauchst du nur zum Selbstbauen, nicht zum Installieren einer fertigen APK.

## 2. Meta-Entwicklerkonto und Verifizierung

Mit „Entwicklerlizenz“ ist hier die Registrierung/Freischaltung des Meta-Kontos gemeint, keine Lizenzdatei, die du auf die Quest kopierst.

1. Öffne die [Meta-Entwicklerregistrierung](https://developers.meta.com/horizon/sign-up/) und melde dich mit dem Konto deiner Quest an.
2. Folge der Registrierung und lies die angezeigten Bedingungen.
3. Erstelle im Developer Dashboard ein Entwicklerteam/eine Organisation oder tritt einer bestehenden bei. Trage zutreffende Angaben ein.
4. Schließe die im Dashboard verlangte Kontoverifizierung ab. Meta nennt je nach Verfahren eine Zahlungsmethode und/oder Zwei-Faktor-Authentifizierung.
5. Falls eine Organisationsverifizierung verlangt wird, folge dem angebotenen Verfahren. Konto- und Organisationsverifizierung sind unterschiedliche Schritte. Bei Einzelentwicklern beschreibt Meta die Admin-Verifizierung als möglichen Weg.

Die aktuellen Anforderungen sind im Dashboard maßgeblich. [Meta: Verifizierungsrichtlinie](https://developers.meta.com/horizon/policy/developer-verification/) · [Meta: Organisation verifizieren](https://developers.meta.com/horizon/resources/publish-organization-verification/)

## 3. Entwicklermodus einschalten

1. Quest einschalten und die Meta Horizon App auf dem Smartphone öffnen.
2. Das gekoppelte Headset über das Headset-Symbol auswählen.
3. **Headset-Einstellungen → Entwicklermodus** öffnen und einschalten.
4. Fehlt der Schalter, zuerst Konto, Teammitgliedschaft und Verifizierung prüfen.

Meta nennt aktuell ein Mindestalter von 18 Jahren und ein Gerät ohne entsprechende Verwaltungssperren. [Aktuelle Meta-Anleitung mit Abbildungen](https://developers.meta.com/horizon/documentation/android-apps/enable-developer-mode/)

## 4. Windows-Treiber und USB-Freigabe

**Windows:** Die offiziellen [Oculus ADB Drivers](https://developers.meta.com/horizon/downloads/package/oculus-adb-drivers/) laden und entpacken. Die enthaltene `.inf`-Datei über das Kontextmenü installieren; unter Windows 11 gegebenenfalls zuerst „Weitere Optionen anzeigen“ öffnen. **macOS:** Dieser Windows-Treiber entfällt.

Quest per USB-Datenkabel verbinden, Headset aufsetzen und die Anfrage **USB-Debugging zulassen** bestätigen. Auf dem eigenen Rechner kann „Von diesem Computer immer zulassen“ gewählt werden. Eine Freigabe für gewöhnlichen Dateizugriff ist nicht dieselbe Debugging-Freigabe.

[Meta: Geräteeinrichtung, Treiber und USB-Freigabe](https://developers.meta.com/horizon/documentation/native/android/mobile-device-setup/)

## 5A. ADB einrichten – direkter Weg

1. [Android SDK Platform-Tools](https://developer.android.com/tools/releases/platform-tools) für Windows oder Mac herunterladen. Das Paket enthält ADB; Android Studio ist dafür nicht erforderlich.
2. ZIP vollständig entpacken, z. B. unter Windows nach `C:\Tools\platform-tools`, auf dem Mac nach `~/Tools/platform-tools`.
3. Terminal beziehungsweise PowerShell öffnen und in diesen Ordner wechseln.

**Windows / PowerShell:**

```powershell
cd C:\Tools\platform-tools
.\adb.exe version
.\adb.exe devices
```

**macOS / Terminal:**

```sh
cd ~/Tools/platform-tools
./adb version
./adb devices
```

Hinter der Seriennummer muss **`device`** stehen. `unauthorized` bedeutet, dass die Debugging-Freigabe fehlt; `offline` bedeutet, dass noch keine nutzbare Verbindung besteht. Bei mehreren Geräten mit `-s SERIENNUMMER` gezielt die Quest auswählen. [ADB-Dokumentation](https://developer.android.com/tools/adb)

In den folgenden Beispielen steht `adb` kurz für `./adb` auf dem Mac beziehungsweise `.\adb.exe` in PowerShell, sofern ADB nicht im PATH liegt.

## 5B. SideQuest installieren – optionale Oberfläche

1. Die [offizielle SideQuest-Einrichtungsseite](https://sidequestvr.com/setup-howto) öffnen.
2. Die Desktop-Ausgabe mit **Advanced Installer / Installation lokaler APK-Dateien** für dein Betriebssystem wählen und installieren. Die Varianten und Bezeichnungen können sich ändern.
3. SideQuest starten, Quest per USB verbinden und die Debugging-Anfrage im Headset bestätigen.
4. Prüfen, ob SideQuest das Gerät als verbunden/autorisiert anzeigt.
5. Zum Installieren unserer APK die Funktion zum **Installieren einer APK vom Computer** wählen und die Projekt-APK auswählen.
6. Warten, bis SideQuest die erfolgreiche Installation meldet.

SideQuest installiert unsere Originalspieldaten nicht automatisch. Dafür folgt Schritt 7. Das Projekt muss nicht im SideQuest-Katalog gelistet sein, um eine lokale APK zu installieren.

[SideQuest: Advanced-Installer-Anleitung](https://medium.com/sidequestvr/how-to-use-sidequests-advanced-installer-42a3d67f91b8) · [SideQuest: Übersicht der Varianten](https://medium.com/sidequestvr/sidequest-for-meta-quest-682ed434aeb1)

## 6. Projekt-APK beziehen und installieren

Unter [Releases](https://github.com/friedensbringer-peacemaker/xr.rttr/releases) nach einer bereitgestellten Quest-APK schauen. Falls noch keine Vorabversion bereitgestellt ist, nach der [Build-Anleitung](../BUILD.md) selbst bauen. Der automatische GitHub-Download „Source code“ ist keine installierbare App.

Die eigene Build-Ausgabe heißt `android/app/build/outputs/apk/debug/xr.rttr-Quest.apk`. Für ADB beispielsweise:

```sh
adb install -r "/Pfad/zur/xr.rttr-Quest.apk"
```

Warte auf `Success`. Wer Schritt 5B verwendet hat, muss die APK nicht zusätzlich mit ADB installieren. Vor jedem Update das laufende Spiel speichern.

## 7. Eigene Siedler-II-Daten übertragen

1. **The Settlers II Gold Edition** aus der eigenen Bibliothek installieren, beispielsweise von [GOG](https://www.gog.com/en/game/the_settlers_2_gold_edition).
2. Den installierten Spieleordner öffnen und darin `DATA` und `GFX` suchen. Nicht den GOG-Installer selbst auf die Quest kopieren.
3. Die Ordner mit ADB übertragen. Die Beispielpfade durch die eigenen Pfade ersetzen:

```sh
adb shell mkdir -p /sdcard/RTTR/S2
adb push "/Pfad/zur/Gold-Edition/DATA" /sdcard/RTTR/S2/
adb push "/Pfad/zur/Gold-Edition/GFX" /sdcard/RTTR/S2/
adb shell ls /sdcard/RTTR/S2
```

Unter PowerShell funktionieren Anführungszeichen ebenso, beispielsweise `"C:\Spiele\Siedler2\DATA"`. Das Ergebnis muss `S2/DATA` und `S2/GFX` sein, nicht `S2/DATA/DATA`.

Bei vorhandenen Daten überschreibt `adb push` gleichnamige Dateien. Für die erste Einrichtung sind die Zielordner leer; bei einem späteren Datenwechsel zuerst sichern. Wenn das Repository und Python vorhanden sind, kann stattdessen unser Hilfsskript verwendet werden: Es verweigert bestehende Daten standardmäßig.

```sh
python3 scripts/quest.py import-data "/Pfad/zur/Gold-Edition"
```

Für dieses Skript ADB in den PATH aufnehmen oder die Variable `ADB` auf die ausführbare Datei setzen. Unter Windows `py -3` verwenden. Ein vollständiger automatischer Kompatibilitätstest ist noch nicht enthalten.

macOS-Hinweis: Eine vorhandene passende Installation lässt sich verwenden. Die Verarbeitung beliebiger Windows-GOG-Installer auf einem Mac ist noch kein Bestandteil unseres Projekts. Eine entsprechende Importfunktion ist geplant; keine automatische Extraktion versprechen.

## 8. Erststart und Kontrolle

1. App regulär starten:

```sh
adb shell am start -n xr.rttr/org.s25rttr.sdl.GameStartActivity
```

2. In der Einrichtung den RTTR-Datenordner `/sdcard/RTTR` und als Originaldatenordner `/sdcard/RTTR/S2` verwenden, soweit abgefragt. Die erforderliche Speicherberechtigung erteilen.
3. Controller bereithalten und eine Controller-Aufforderung im Headset bestätigen.
4. Das Menü sollte als Spielfenster erscheinen. Controllerstrahl und Klick testen; anschließend ein Spiel starten.
5. Speichern und den Spielstand erneut laden, um den vollständigen Ablauf zu prüfen.

Spätere Starts sind über die Bibliothek unter **Unbekannte Quellen / Unknown Sources** möglich; die App heißt dort **xr.rttr**. [Meta: selbst installierte Apps finden](https://developers.meta.com/horizon/documentation/android-apps/enable-developer-mode/)

Bedienung: [Controllerbelegung, Panel und Geschwindigkeit](Quest-Bedienung.md). Die Details der ersten Dateiauswahl können sich zwischen Builds unterscheiden; ein vollständig geführter Desktop-Assistent ist noch nicht implementiert.

## 9. Updates und Sicherung

Vor einem Update im Spiel speichern und die Benutzerdaten extern sichern, beispielsweise in einen neuen lokalen Ordner:

```sh
adb pull /sdcard/RTTR/.s25rttr "RTTR-Backup-2026-09-26"
```

Für jede Sicherung einen neuen Ordnernamen verwenden und danach kontrollieren, ob die Dateien angekommen sind. Bei Versions-/Signaturfehlern nicht einfach deinstallieren: Das kann Daten entfernen. Details zur Debug-Signierung stehen in der [Build-Anleitung](../BUILD.md#signierung-und-updates).

## 10. Häufige Probleme

| Beobachtung | Nächster Schritt |
| --- | --- |
| Kein Entwicklermodus-Schalter | Richtiges Meta-Konto, Team/Organisation und Verifizierungsstatus prüfen; aktuelle Meta-Anleitung öffnen |
| `adb` wird nicht gefunden | Im entpackten `platform-tools`-Ordner arbeiten; `./adb` bzw. `.\adb.exe` verwenden |
| Leere Geräteliste | Datenkabel, anderen USB-Port und unter Windows den Treiber prüfen |
| `unauthorized` | Headset aufsetzen, entsperren und Debugging-Freigabe bestätigen |
| `offline` oder wechselnde Verbindung | Kabel neu verbinden; konkurrierende ADB-/SideQuest-/Developer-Hub-Instanzen schließen |
| Mehrere Geräte | Mit `adb -s SERIENNUMMER …` die Quest auswählen |
| App installiert, aber nicht auffindbar | Bibliothek „Unbekannte Quellen“ prüfen; gegebenenfalls über `GameStartActivity` starten |
| Originaldaten fehlen | Verzeichnisstruktur `S2/DATA` und `S2/GFX` sowie Speicherberechtigung prüfen |
| Signaturkonflikt beim Update | Passend signierte APK verwenden; vor jeder Deinstallation sichern |
| Startfehler nach direktem Aufruf von `SDLActivity` | Regulären Einstieg `GameStartActivity` verwenden |

Bei instabilem ADB-Zugriff empfiehlt Meta, mehrere installierte ADB-Versionen zu prüfen: [ADB-Verbindungsdiagnose](https://developers.meta.com/horizon/documentation/android-apps/enable-developer-mode/).

## Videos und weiterführende Hilfe

- [SideQuest VR: „How to Enable Developer Mode on Quest 3 to Use SideQuest“](https://www.youtube.com/watch?v=3n1T59v4HTY) – offizieller SideQuest-Kanal, 5. März 2024, Englisch. Zeigt den grundsätzlichen Ablauf; die heutige Konto-/Teamprüfung kann anders aussehen.
- [Meta: Geräteeinrichtung mit Abbildungen und eingebettetem Video](https://developers.meta.com/horizon/documentation/native/android/mobile-device-setup/) – aktuelle Referenz für die Entwicklermodus-Schritte.
- [SideQuest: aktuelle Installationshilfe](https://sidequestvr.com/setup-howto) – bei geänderter SideQuest-Oberfläche hier beginnen.
- [Android: ausführliche ADB-Befehlsreferenz](https://developer.android.com/tools/adb) – Installation, Dateikopien und Geräteauswahl.

Die Links wurden für diese Dokumentation recherchiert. Der gesamte Ablauf wurde noch nicht mit einem frisch registrierten Meta-Konto oder auf Windows durchgespielt; aktuelle Kontovorgaben und UI-Abweichungen sind bei einem solchen Test nachzutragen.
