# Schnellstart: Zentrale verbinden

Nach der Erstellung der ersten Welt folgt der nächste Schritt: Traintastic mit der **Zentrale / dem Digitalsystem** verbinden. Dazu wird eine **Schnittstelle (Interface)** angelegt, die als Verbindung zwischen Traintastic und der Hardware dient.

## Schritt 1: Interface-Liste öffnen

1. Sicherstellen, dass der **Bearbeitungsmodus** aktiv ist (![Stift](../assets/images/icons/light/edit.png#only-light)![Stift](../assets/images/icons/dark/edit.png#only-dark) oben rechts).
2. Im Hauptmenü zu **Objekte → Hardware → Schnittstellen** gehen.
3. Es öffnet sich der *Schnittstellen*-Dialog. Nach einer frischen Installation ist diese Liste leer.

## Schritt 2: Einrichtungsassistent starten

1. Auf die ![Plus](../assets/images/icons/dark/circle/add.png)-Schaltfläche klicken, um eine neue Schnittstelle hinzuzufügen.
2. Im Menü **„Einrichtung über Assistent“** auswählen.  
   - Das Menü zeigt auch alle unterstützten Schnittstellentypen direkt an, diese sind jedoch für erfahrene Nutzer gedacht, die die Konfiguration manuell vornehmen möchten.

## Schritt 3: System auswählen

Im Assistenten das **Digitalsystem / die Zentrale** aus der Hardwareliste auswählen.

- Der Assistent enthält die gängigsten Systeme.
- Nicht alle Zentralen sind enthalten, viele weitere Modelle können jedoch manuell über das *Schnittstellen*-Menü konfiguriert werden.  
  Eine vollständige Übersicht getesteter Systeme befindet sich im [Anhang unterstützte Hardware](../appendix/supported-hardware/command-stations/index.md).

!!! note
    Falls die eigene Zentrale nicht im Assistenten erscheint, lohnt sich ein Blick ins [Community-Forum](https://discourse.traintastic.org).  
    Dort gibt es oft bereits Erfahrungen anderer Nutzer, und Rückmeldungen helfen dabei, den Assistenten weiter auszubauen.

Je nach Auswahl fragt Traintastic zusätzliche Informationen ab, z. B.:

- **Verbindungsart**: seriell, USB oder Netzwerk (Ethernet oder WLAN)
- **Gerät oder Port**: z. B. COM-Port (Windows), `/dev/ttyUSB0` (Linux) oder eine IP-Adresse

!!! tip "WLAN verwenden"
    WLAN wird unterstützt, ist aber aus Stabilitätsgründen nicht empfohlen.

Nach Abschluss des Assistenten:

- erscheint die neue Schnittstelle in der *Schnittstellen*-Liste
- in der Statusleiste wird ein **Statussymbol** angezeigt (rechte Fensterseite):
    - ![Grau](../assets/images/icons/dark/interface_state.offline.png) — Offline
    - ![Lila](../assets/images/icons/dark/interface_state.initializing.png) — Initialisierung
    - ![Grün](../assets/images/icons/dark/interface_state.online.png) — Online/verbunden
    - ![Rot](../assets/images/icons/dark/interface_state.error.png) — Fehler aufgetreten

Bei mehreren Schnittstellen hat jede ihr eigenes Statussymbol.

## Schritt 4: Verbinden und testen

1. Die ![Verbinden](../assets/images/icons/light/offline.png#only-light)![Verbinden](../assets/images/icons/dark/offline.png#only-dark)-Schaltfläche drücken  
   oder über das Menü: **Welt → Verbindung → Verbinden**
2. Der Status ändert sich:
     - ![Lila](../assets/images/icons/dark/interface_state.initializing.png) — während der Initialisierung (einige Sekunden)
     - ![Grün](../assets/images/icons/dark/interface_state.online.png) — Verbindung erfolgreich
     - ![Rot](../assets/images/icons/dark/interface_state.error.png) — Fehler aufgetreten

Bei einem Fehler:

- das **Server-Log** öffnen (Taste ++F12++ oder **Ansicht → Server-Log**) für Details
- siehe [Häufige Schnittstellenfehler](../troubleshooting/interface-connection-errors.md)
- prüfen, ob das System unterstützt wird: [Unterstützte Hardware](../appendix/supported-hardware/command-stations/index.md)

Wenn die Verbindung funktioniert:

Die Gleisspannung über die Schaltfläche
![Power aus](../assets/images/icons/light/power_off.png#only-light)![Power aus](../assets/images/icons/dark/power_off.png#only-dark) /
![Power ein](../assets/images/icons/light/power_on.png#only-light)![Power ein](../assets/images/icons/dark/power_on.png#only-dark)
umschalten.  
Reagiert die Zentrale, ist die Verbindung erfolgreich hergestellt.

---

Mit verbundener Zentrale kann im nächsten Schritt [eine Lokomotive hinzugefügt und gesteuert werden](trains.md).