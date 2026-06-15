# CBUS/VLCB Referenz

CBUS ist ein Layout-Control-Bus, entwickelt von Mike Bolton und Gil Fuchs, Mitgliedern der Model Electronic Railway Group (MERG).
CBUS verwendet das Controller Area Network (CAN) zur Kommunikation zwischen CBUS-Modulen.

VLCB ist eine abwärtskompatible Erweiterung von CBUS, ebenfalls von MERG-Mitgliedern entwickelt. Sie ergänzt zusätzliche Befehle und führt ein strengeres Prioritätssystem für Nachrichten ein.

Dieser Anhang beschreibt **nicht** das CBUS/VLCB-Protokoll selbst.
Stattdessen beschreibt er **wie Traintastic CBUS/VLCB implementiert und verwendet** sowie welche Protokollnachrichten unterstützt werden.
Er richtet sich an fortgeschrittene Anwender, die bereits mit den Grundlagen des CBUS/VLCB-Protokolls vertraut sind.

## Unterstützte Hardware

*TODO: in Entwicklung*

## Nachrichtenunterstützung

### Allgemein
- Node-Erkennung: `QNN`, `PNN` – teilweise unterstützt
- Gleisspannung ein/aus: `TOF`, `TON`, `RTOF`, `RTON` – unterstützt
- Notstopp: `ESTOP`, `RESTP` – unterstützt
- Command-Station-Status: `RSTAT`, `STAT` – teilweise unterstützt

### Lokomotivsteuerung
- Session-Management: `GLOC`, `KLOC`, `DKEEP`, `PLOC`, `ERR` – teilweise unterstützt
- Geschwindigkeits-/Richtungssteuerung: `DSPD`, `STMOD` – unterstützt
- Funktionssteuerung: `DFNON`, `DFNOF` – unterstützt
- Consisting (Mehrfachtraktion): – nicht unterstützt

### Weichen, Signale und Ausgänge
- Kurze Events: `ASON`, `ASOF` – unterstützt
- Lange Events: `ACON`, `ACOF` – unterstützt

### Rückmeldesensoren
- Kurze Events: `ASON`, `ASOF` – unterstützt
- Lange Events: `ACON`, `ACOF` – unterstützt

### Sonstiges
- Senden roher DCC-Pakete: `RDCC3`, `RDCC4`, `RDCC5`, `RDCC6` – unterstützt

## Debugging und Monitoring

Traintastic bietet eine Debug-Option für CBUS/VLCB, die den gesamten Busverkehr protokolliert.
Nachrichten werden im **Hexadezimalformat** angezeigt, und für viele Nachrichtentypen wird zusätzlich eine menschenlesbare Beschreibung des Inhalts angezeigt.

Dies ist nützlich für:

- Diagnose von Kompatibilitätsproblemen mit bestimmten Modulen
- Überprüfung, ob Nachrichten korrekt gesendet und empfangen werden

### Senden von Rohnachrichten

Über [**Lua-Skripting**](../advanced/scripting-basics.md) ist es außerdem möglich:

- **Roh-CBUS/VLCB-Nachrichten** zu senden, siehe [`send()`](lua/object/cbusinterface.md#send)
- **Roh-DCC-Track-Kommandos** (`RDCCn`) zu senden, siehe [`send_dcc()`](lua/object/cbusinterface.md#send_dcc)

!!! warning "Mit Vorsicht verwenden!"
    - Diese Nachrichten umgehen die normale Verarbeitung von Traintastic.
    - Ein gutes Verständnis von CBUS/VLCB und DCC ist erforderlich, um Konflikte zu vermeiden.
    - Es können Nebenwirkungen auftreten, die Traintastic nicht erkennt oder nicht verwalten kann.

---

<small>
CBUS® ist eine eingetragene Marke von Dr. Mike Bolton. \
CBUS® Protokolldokumente sind urheberrechtlich geschützt von Mike Bolton und Gil Fuchs.
</small>
```
