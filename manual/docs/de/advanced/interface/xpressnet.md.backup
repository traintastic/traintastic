```markdown id="xpressnet_de"
# XpressNet Schnittstellenkonfiguration

Diese Seite beschreibt die Konfiguration einer XpressNet-Zentrale in Traintastic.

!!! tip
    Viele Zentralen können über den **Einrichtungsassistenten** hinzugefügt werden, der Schritt für Schritt durch die Konfiguration führt.  
    Siehe [Schnellstart: Zentrale verbinden](../../quickstart/command-station.md) für Details.

## Unterstützte Verbindungsarten

XpressNet-Zentralen können auf zwei Arten mit Traintastic verbunden werden:

- **Seriell (RS-232/USB)** – Direkte Kabelverbindung, häufig über USB-zu-Seriell-Adapter  
  Einige Geräte stellen Presets mit vordefinierter Baudrate und Flusskontrolle bereit
- **Netzwerk** – TCP/IP-Verbindung über Ethernet oder WLAN

## Verbindungseinstellungen

Je nach Verbindungsart stehen folgende Optionen zur Verfügung:

### Serielle Verbindungen

- **Serielle Schnittstelle (Typ)** – Preset für die verwendete Hardware (z. B. RoSoft S88XPressNetLI)  
  Dieses Preset legt verfügbare Baudraten und Flusskontrolle fest oder schränkt sie ein
- **Gerät** – Pfad zum seriellen Gerät (z. B. `COM3` unter Windows oder `/dev/ttyUSB0` unter Linux)
- **Baudrate** – Übertragungsgeschwindigkeit, durch den gewählten Schnittstellentyp festgelegt
- **Flusskontrolle** – Hardware-/Software-Flusskontrolle, meist *Keine*, außer vom Gerät gefordert
- **S88 Startadresse** – (nur RoSoft S88XPressNetLI) Erste Adresse der angeschlossenen S88-Rückmeldemodule
- **S88 Modulanzahl** – (nur RoSoft S88XPressNetLI) Anzahl der angeschlossenen S88-Module

### Netzwerkverbindungen

- **Hostname** – IP-Adresse oder Hostname der Zentrale
- **Port** – TCP-Portnummer

## XpressNet Einstellungen

Zusätzliche Optionen zur Feinabstimmung:

### Zentralen-Preset

- **Zentrale** – Auswahl des Zentralentyps  
  Dieses Preset passt automatisch weitere Einstellungen an die jeweilige Zentrale an.  
  Die Auswahl **Custom** erlaubt eine manuelle Konfiguration aller Parameter.

### Loksteuerung

- **Not-Halt-Lokbefehl verwenden** – Aktiviert den speziellen XpressNet-Befehl für Lok-Not-Halt
- **Roco F13–F20 Befehl verwenden** – Aktiviert den Roco-spezifischen erweiterten Funktionsbefehl für F13–F20

### Zubehörsteuerung

- **Roco Zubehöradressierung verwenden** – Aktiviert die Roco-Adressierung für Zubehör (Alternative zur Standard-XpressNet-Adressierung)

### Debugging

- **Debug-Log RX/TX** – Protokolliert den gesamten XpressNet-Datenverkehr (Hex + Beschreibung)

!!! tip "Hilfe bei Problemen"
    Bei Problemen mit der Konfiguration oder unerwartetem Verhalten hilft ein Blick ins [Community-Forum](https://discourse.traintastic.org).  
    Das Teilen von Konfigurationen und Erfahrungen hilft auch anderen und verbessert Traintastic.
```
