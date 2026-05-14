```markdown id="marklin_can_de"
# Märklin CAN Schnittstellenkonfiguration

Diese Seite beschreibt die Konfiguration einer Märklin CAN-Zentrale in Traintastic.

!!! tip
    Märklin CS2/CS3 können über den **Einrichtungsassistenten** hinzugefügt werden, der Schritt für Schritt durch die Konfiguration führt.  
    Siehe [Schnellstart: Zentrale verbinden](../../quickstart/command-station.md) für Details.

## Unterstützte Verbindungsarten

Märklin CAN-Zentralen können auf zwei Arten mit Traintastic verbunden werden:

- **Netzwerk (TCP/IP)** – Empfohlen. Verbindung über Ethernet oder WLAN mit dem Märklin CAN-Protokoll
- **Seriell (RS-232/USB)** – Direkte Kabelverbindung, häufig über USB-zu-Seriell-Adapter

## Verbindungseinstellungen

Je nach Verbindungsart stehen folgende Optionen zur Verfügung:

### Netzwerkverbindungen
- **Hostname** – IP-Adresse oder Hostname der Zentrale
- **Port** – TCP-Portnummer (standardmäßig abhängig vom Zentraltentyp)

### Serielle Verbindungen
- **Gerät** – Pfad zum seriellen Gerät (z. B. `COM3` unter Windows oder `/dev/ttyUSB0` unter Linux)
- **Baudrate** – Übertragungsgeschwindigkeit, abhängig vom CAN-Bus-Interface
- **Flusskontrolle** – Hardware-/Software-Flusskontrolle, meist *Keine*, außer vom Interface gefordert

!!! note
    Serielle Verbindungen werden bei **CAN-Bus-Interfaces** verwendet, die den Märklin CAN-Bus über RS-232 oder USB an den Computer weiterleiten.  
    Dies ist etwas anderes als eine direkte Verbindung zu einer CS2/CS3, die über Netzwerk erfolgen sollte.

## Märklin CAN Einstellungen

Zusätzliche Optionen zur Feinabstimmung:

### Allgemein
- **Standard-Schaltzeit** – Standarddauer (ms) für Weichen- und Zubehörbefehle

### Identifikation
- **Node UID** – Eindeutige Kennung des CAN-Knotens
- **Node-Seriennummer** – Seriennummer des CAN-Knotens

Traintastic registriert sich im Märklin CAN-Bus als eigener Knoten. Bei Konflikten können UID und Seriennummer angepasst werden.

### Debugging
- **Debug-Log RX/TX** – Protokolliert den gesamten CAN-Bus-Verkehr (Hex + Beschreibung)
- **Debug Statusdaten-Konfiguration** – Protokolliert CAN-Status- und Konfigurationsmeldungen
- **Debug Konfigurations-Stream** – Protokolliert den Roh-Konfigurationsdatenstrom

!!! tip "Hilfe bei Problemen"
    Bei Problemen mit der Konfiguration oder unerwartetem Verhalten hilft ein Blick ins [Community-Forum](https://discourse.traintastic.org).  
    Das Teilen von Konfigurationen und Erfahrungen hilft auch anderen und verbessert Traintastic.
```
