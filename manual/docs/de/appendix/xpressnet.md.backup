```markdown id="xpressnet_reference_de"
# XpressNet Referenz

XpressNet (ursprünglich X-Bus genannt) ist ein Kommunikationsbus, der von der Lenz Elektronik GmbH entwickelt wurde.  
Er wird von Lenz und mehreren anderen Herstellern verwendet und ermöglicht den Mischbetrieb von Geräten unterschiedlicher Anbieter.  
Dieser Anhang beschreibt die Implementierungsdetails von XpressNet in Traintastic.

## Unterstützte Hardware

Traintastic unterstützt eine Vielzahl von Zentralen und Interfaces mit XpressNet-Funktionalität.  
Siehe den Abschnitt [Unterstützte Hardware](supported-hardware/index.md) für eine vollständige und aktuelle Liste.

## Nachrichtenunterstützung

### Stromversorgung
- Strom ein – Unterstützt
- Strom aus – Unterstützt

### Loksteuerung
- Not-Halt aller Lokomotiven – Unterstützt
- Not-Halt einer Lokomotive – Unterstützt
- Geschwindigkeit & Fahrtrichtung: 14/27/28/128 Fahrstufen – Unterstützt
- Funktionen F0–F28 – Unterstützt
- Roco MultiMAUS Funktionen F13–F20 – Unterstützt (basierend auf Analyse des MultiMAUS-Datenverkehrs)
- Mehrfachtraktionen (Consists) – **Nicht unterstützt**, Traintastic verwaltet Lokomotiven einzeln

### Weichen, Signale und Ausgänge
- Zubehörsteuerung – Unterstützt

### Rückmeldesensoren
- Rückmeldeinformationen – Unterstützt

### Echtzeituhr
- OpenDCC proprietäre Erweiterung – Nicht unterstützt

### Programmierung
- Decoder-Programmierung – Geplant, derzeit nicht unterstützt

## Debugging und Überwachung

Traintastic bietet eine Debug-Option für XpressNet, die den gesamten Busverkehr protokolliert.  
Die Nachrichten werden im **Hexadezimalformat** dargestellt, zusätzlich wird für viele Nachrichtentypen eine menschenlesbare Beschreibung angezeigt.

Dies ist nützlich für:

- Analyse von Kompatibilitätsproblemen mit bestimmten Geräten
- Überprüfung der korrekten Übertragung und Verarbeitung von Nachrichten
- Untersuchung herstellerspezifischer oder nicht dokumentierter XpressNet-Erweiterungen

---

!!! footnote
    Vollständige Protokolldetails finden sich im offiziellen [XpressNet-Spezifikationsdokument im *23151 Interface LAN und USB* Handbuch (PDF, Deutsch)](https://www.lenz-elektronik.de/media/37/8b/2f/1734009949/b_23151.pdf), veröffentlicht von Lenz.
```
