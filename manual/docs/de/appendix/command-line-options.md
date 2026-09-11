# Kommandozeilenoptionen

Client und Server unterstützen verschiedene Kommandozeilenoptionen, um zu steuern, wie die Anwendungen starten und wo Daten gespeichert werden.
Diese Optionen sind vor allem für PCs gedacht, die ausschließlich zur Steuerung des Layouts verwendet werden.

## Traintastic Client

| Kurz                  | Lang                          | Beschreibung                     |
|----------------------|-------------------------------|----------------------------------|
| `-h`                 | `--help`                      | Hilfe anzeigen                  |
| `-v`                 | `--version`                   | Versionsinformationen anzeigen  |
|                      | `--fullscreen`                | Anwendung im Vollbild starten   |
| `-c <hostname[:port]>` | `--connect <hostname[:port]>` | Mit Server verbinden            |

## Traintastic Server

Die verfügbaren Kommandozeilenoptionen des Servers hängen vom verwendeten Betriebssystem ab.
Die Spalte **Betriebssystem** zeigt, ob eine Option auf **allen Systemen**, **Windows**, **Linux** und/oder **macOS** verfügbar ist.

| Kurz        | Lang              | Beschreibung                              | Betriebssystem |
|-------------|-------------------|-------------------------------------------|----------------|
| `-h`        | `--help`          | Hilfe anzeigen und beenden                | Alle           |
| `-v`        | `--version`       | Versionsinformationen ausgeben und beenden| Alle           |
| `-D PATH`   | `--datadir PATH`  | Datenverzeichnis                         | Alle           |
| `-W UUID`   | `--world UUID`    | Zu ladende World-UUID                    | Alle           |
|             | `--simulate`      | Simulation nach dem Laden der Welt aktivieren | Alle      |
|             | `--online`        | Kommunikation nach dem Laden aktivieren  | Alle           |
|             | `--power`         | Gleisspannung nach dem Laden aktivieren  | Alle           |
|             | `--run`           | Nach dem Laden starten                   | Alle           |
|             | `--tray`          | Anwendung im System-Tray ausführen       | Windows        |
| `-d`        | `--daemonize`     | Als Hintergrunddienst ausführen          | Linux, macOS   |
| `-u NAME`   | `--user NAME`     | Als Benutzer ausführen                   | Linux, macOS   |
| `-g NAME`   | `--group NAME`    | Als Gruppe ausführen                     | Linux, macOS   |
| `-p [FILENAME]` | `--pidfile [FILENAME]` | PID-Datei schreiben (Standard: `/run/traintastic-server.pid`) | Linux, macOS |

!!! note
    Die Optionen `--simulate`, `--online`, `--power` und `--run` gelten nur für die beim Start geladene Welt.

!!! note
    Die Option `--run` erfordert `--power`. `--power` muss gesetzt sein, damit `--run` funktioniert.

### Datenverzeichnis

Das *Datenverzeichnis* ist der Speicherort, in dem der Traintastic Server alle Daten ablegt, z. B. Einstellungen, Welten, Logdateien und Backups.
Der Standardpfad hängt vom Betriebssystem ab:

- **Windows**: `%LOCALAPPDATA%\\traintastic\\server`, z. B. `C:\\Users\\reinder\\AppData\\Local\\traintastic\\server`
- **Linux**:
    - Bei normalem Benutzer: `~/.config/traintastic-server`, z. B. `/home/reinder/.config/traintastic-server`
    - Bei systemd-Dienst: `/var/opt/traintastic`

!!! note
    Der Server speichert Daten pro Benutzer. Wenn mehrere Benutzer dieselben Einstellungen, Welten, Logdateien oder Backups verwenden sollen, muss das Datenverzeichnis auf einen gemeinsamen, für alle schreibbaren Pfad gesetzt werden.
