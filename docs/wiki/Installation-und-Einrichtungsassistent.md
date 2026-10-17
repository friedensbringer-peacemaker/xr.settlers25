# Installation und Einrichtungsassistent

Für die heutige manuelle Einrichtung gibt es die [vollständige Schritt-für-Schritt-Anleitung](Ersteinrichtung-Schritt-fuer-Schritt.md).

## Ziel: drei Schritte auf Windows und macOS
**Konzept, noch nicht implementiert.** Nutzer sollen keine Entwicklungsumgebung benötigen und nicht selbst kompilieren müssen.

1. **Spiel auswählen:** vorhandenen Siedler-II-Ordner auswählen oder hineinziehen. Benötigte Dateien prüfen, verständlich erklären, was fehlt.
2. **Quest verbinden:** USB-Verbindung und ADB-Erreichbarkeit prüfen. Bei fehlender Freigabe genau zeigen, welcher Schritt im Headset nötig ist.
3. **Auf Quest installieren:** eine fertige, zum Projekt gehörende APK und die geprüften Originaldaten übertragen. Ergebnis prüfen und App starten.

Das Programm soll ADB mitbringen; Bereitstellung und Lizenzhinweise sind bei der Umsetzung zu prüfen. Unter Windows kann zusätzlich eine Treibereinrichtung erforderlich sein. Unterschiedliche Originaldaten erfordern voraussichtlich keinen eigenen APK-Build; das muss anhand unterstützter Datensätze geprüft werden.

## Spätere Funktionen
- App aktualisieren, ohne Originaldaten oder Spielstände zu überschreiben.
- Spielstände gezielt sichern und wiederherstellen.
- Installierte Version und Übertragungsergebnis anzeigen.
- Bei Fehlern verständliche Diagnose statt Terminalbefehlen anbieten.

Vor einer Neuinstallation oder Datenersetzung soll der Assistent bestehende Daten erkennen und deren Erhalt bzw. Sicherung sicherstellen. Diese Schutzfunktionen sind Anforderungen, noch keine Zusage über ein vorhandenes Werkzeug.

## Einmalige Quest-Voraussetzungen
Für die ADB-Installation werden Entwicklermodus und die bestätigte USB-Debugging-Freigabe im Headset benötigt. Der Assistent kann dabei anleiten, die Bestätigung aber nicht ersetzen. Aktuelle Schritte: [Meta-Geräteeinrichtung](https://developers.meta.com/horizon/documentation/native/android/mobile-device-setup/).

## Aktueller Entwicklerweg
Noch erfolgt die Einrichtung manuell mit ADB und lokal gebauter APK. Das Anwendungspaket heißt `xr.rttr` (Java-Namespace weiterhin `org.s25rttr.sdl`); Originaldaten liegen in `/sdcard/RTTR/S2`, Benutzerdaten in `/sdcard/RTTR/.s25rttr`.

```sh
adb install -r <Quest-APK>
adb shell am start -n xr.rttr/org.s25rttr.sdl.GameStartActivity
```

Ein Kaltstart muss über `GameStartActivity` erfolgen. Direkter Start von `SDLActivity` kann die notwendige Prozessumgebung umgehen. Eine APK-Installation beendet gegebenenfalls das laufende Spiel; vorher speichern. Der Befehl allein ersetzt weder Datenimport noch vollständige Ersteinrichtung.

## Alternative: Installation im Browser
Ein WebUSB-basierter Assistent ist eine spätere Option. Er könnte einen separaten Desktop-Download vermeiden, benötigt aber Prüfungen der Browser-, Betriebssystem- und Treiberkompatibilität. Für die erste Umsetzung wird der Desktop-Assistent bevorzugt. [WebUSB-Dokumentation](https://developer.chrome.com/docs/capabilities/usb)
