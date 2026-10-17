# Start-Prompts für beliebige KI-Assistenten

Für ChatGPT/GPT, Claude, Codex oder einen anderen Agenten geeignet. Mit lokalem Repository-Zugriff die Dateien lesen lassen. In einem reinen Chat folgende Dateien als Text/Anhänge bereitstellen: PROJECT-CONTEXT.md, SETUP-RUNBOOK.md, README.md und die passende Windows-/macOS-Anleitung. Den Platzhalterpfad ersetzen. Ein privater GitHub-Link allein gewährt dem Assistenten keinen Zugriff.

## Konfiguration und Build mit Rechnerzugriff

```text
Hilf mir, xr.rttr auf meinem Rechner zu konfigurieren und eine Quest-3-APK zu bauen.
Der Checkout liegt unter: <ABSOLUTER PROJEKTPFAD>.
Lies zuerst AGENTS.md, README.md, docs/agents/PROJECT-CONTEXT.md und docs/agents/SETUP-RUNBOOK.md sowie die passende Anleitung unter docs/setup/.
Prüfe Betriebssystem und bereits vorhandene Werkzeuge. Erhalte vorhandene Änderungen und verwende passende installierte Werkzeuge wieder. Führe die lokale Preflight-Prüfung aus, behebe die konkret gemeldeten Voraussetzungen und baue anschließend die APK. Verwende keine Originalspieldaten zum Build und veröffentliche keine Zugangsdaten.
Überlasse mir Kontoanmeldungen, Lizenzannahmen und Headset-Bestätigungen. Melde am Ende den APK-Pfad, die ausgeführten Prüfungen und noch offene Punkte. Installiere die APK auf einem Headset erst, wenn ich auch die Geräteinstallation beauftrage.
```

## Schritt-für-Schritt-Hilfe ohne Rechnerzugriff

```text
Ich möchte xr.rttr einrichten und bin wenig technisch erfahren. Die angehängten Projektdateien erklären Aufbau und Ablauf.
Frage zuerst nach meinem Betriebssystem und danach, ob ich eine fertige APK installieren oder selbst bauen möchte. Gib mir jeweils nur den nächsten überschaubaren Schritt mit einem exakt passenden kopierbaren Befehlsblock. Warte auf meine Rückmeldung, bevor du abhängige Schritte vorgibst. Erkläre, welche Ausgabe ich erwarten soll. Verlange keine Passwörter oder Tokens. Behaupte nicht, Befehle selbst ausgeführt zu haben.
```

## Vorhandene APK auf die Quest installieren

```text
Hilf mir, die vorhandene xr.rttr-APK auf meiner Quest 3 zu installieren und meine eigenen Siedler-II-Gold-Daten zu übertragen.
Lies docs/agents/PROJECT-CONTEXT.md, docs/agents/SETUP-RUNBOOK.md und docs/wiki/Ersteinrichtung-Schritt-fuer-Schritt.md. Prüfe zuerst die Geräteverbindung und vorhandene Daten. Sichere bei einer bestehenden Installation die Spielstände vor Änderungen und prüfe das Backup. Verwende das beabsichtigte Gerät; deinstalliere nichts automatisch und überschreibe keine vorhandenen Originaldaten ohne dazu passenden Auftrag.
Starte über GameStartActivity und leite mich bei der Sichtprüfung im Headset an. Eine erfolgreiche ADB-Installation allein ist noch kein erfolgreicher Spieltest.
```

## Fehler untersuchen

```text
Untersuche diesen Fehler bei xr.rttr anhand des beigefügten Protokolls und der Projektdateien.
Unterscheide zwischen belegten Ursachen und Vermutungen. Frage nur fehlende relevante Angaben nach. Schlage zuerst eine kleine gezielte Korrektur vor; aktualisiere nicht pauschal alle Werkzeuge oder Abhängigkeiten. Erhalte meine Dateien und Spielstände. Prüfe danach, ob die ursprüngliche Fehlermeldung behoben ist. Nenne zum Abschluss die Änderung, den Nachweis und verbleibende Unsicherheiten.
```
