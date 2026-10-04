# LocoNet Schnittstellenkonfiguration

Diese Seite beschreibt die Konfiguration einer LocoNet-Zentrale in Traintastic.

!!! tip
    Viele Zentralen können über den **Einrichtungsassistenten** hinzugefügt werden, der Schritt für Schritt durch die Konfiguration führt.  
    Siehe [Schnellstart: Zentrale verbinden](../../quickstart/command-station.md) für Details.

## Unterstützte Verbindungsarten

LocoNet-Zentralen können auf verschiedene Arten mit Traintastic verbunden werden:

- **Seriell (RS-232/USB)** – Direkte Kabelverbindung, häufig über USB-zu-Seriell-Adapter
- **Netzwerk – Binäres Protokoll** – TCP/IP-Verbindung über das LocoNet-Binärprotokoll
- **Netzwerk – LBserver-Protokoll** – TCP/IP-Verbindung über das textbasierte LBserver-Protokoll
- **Netzwerk – Z21-Protokoll** – UDP-Verbindung kompatibel zur Z21-Schnittstelle

## Verbindungseinstellungen

Je nach Verbindungsart stehen folgende Optionen zur Verfügung:

### Serielle Verbindungen
- **Gerät** – Pfad zum seriellen Gerät (z. B. `COM3` unter Windows oder `/dev/ttyUSB0` unter Linux)
- **Baudrate** – Übertragungsgeschwindigkeit, muss mit der Konfiguration der Zentrale übereinstimmen
- **Flusskontrolle** – Hardware-/Software-Flusskontrolle, meist *Keine*, außer das Gerät erfordert sie

### Netzwerkverbindungen
- **Hostname** – IP-Adresse oder Hostname der Zentrale
- **Port** – TCP/UDP-Portnummer für das gewählte Protokoll

## LocoNet Einstellungen

Zusätzliche Optionen zur Feinabstimmung:

### Zentralen-Preset

- **Zentrale** – Auswahl des Zentralentyps  
  Dieses Preset passt automatisch Einstellungen wie Timeouts und Slot-Verwaltung an die jeweilige Zentrale an.  
  Die Auswahl **Custom** erlaubt eine manuelle Konfiguration aller Parameter.

### Loksteuerung

- **Lok-Slots** – Anzahl der zu verwaltenden Lok-Slots
- **F9–F28 Funktionen** – Festlegung, wie erweiterte Funktionen (F9–F28) gesteuert werden  
  Standard: Digitrax `OPC_IMM_PACKET`  
  Alternative: Uhlenbrock-Erweiterungsbefehle

### Echtzeituhr (Fast Clock)

- **Fast Clock** – Aktiviert oder deaktiviert die Schnellzeituhr
- **Fast Clock Sync aktiviert** – Synchronisation der Traintastic-Uhr mit der LocoNet-Uhr
- **Fast Clock Sync Intervall** – Intervall (Sekunden) für die Synchronisation

### Erweitert

- **Echo-Timeout** – Zeitlimit (ms) für Echo-Antworten
- **Antwort-Timeout** – Zeitlimit (ms) für Befehlsantworten

### Debugging

- **Debug-Log RX/TX** – Protokolliert den gesamten LocoNet-Datenverkehr (Hex + Beschreibung)
- **PCAP-Aufzeichnung** – Aktiviert Paketaufzeichnung zur Analyse
- **PCAP-Ausgabedatei** – Dateipfad für die gespeicherten Pakete
- **Nur-Mitlauschen-Modus** – Passiver Überwachungsmodus; es werden keine Befehle gesendet

!!! tip "Hilfe bei Problemen"
    Bei Problemen mit der Konfiguration oder unerwartetem Verhalten hilft ein Blick ins [Community-Forum](https://discourse.traintastic.org).  
    Das Teilen von Konfigurationen und Erfahrungen hilft auch anderen und verbessert Traintastic.
