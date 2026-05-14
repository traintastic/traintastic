```markdown id="locointro_de1"
# LocoNet reference

LocoNet ist ein Netzwerkbus, der Anfang der 1990er Jahre von Digitrax Inc. für eigene Produkte entwickelt wurde.
Heute wird er von mehreren Herstellern genutzt, und Geräte verschiedener Anbieter lassen sich in der Regel dank des standardisierten Busses kombinieren.

Dieses Kapitel beschreibt die **Implementierungsdetails von LocoNet in Traintastic**.

Es erklärt **nicht** das LocoNet-Protokoll selbst. Stattdessen wird beschrieben, wie Traintastic LocoNet integriert und welche Nachrichten unterstützt werden.

Dieses Kapitel richtet sich an fortgeschrittene Nutzer, die mit den Grundlagen von LocoNet bereits vertraut sind.
```

---

```markdown id="loconet_impl_de1"
## Unterstützte Hardware

Traintastic unterstützt eine breite Auswahl an Zentraleinheiten und Schnittstellen mit LocoNet-Funktionalität.  
Siehe Abschnitt [Supported hardware](supported-hardware/index.md) für eine vollständige und aktuelle Liste.

---

## Implementierungsphilosophie

Traintastic integriert LocoNet, folgt dabei jedoch seinem eigenen internen Steuerungsmodell:

- **Direkte Loksteuerung**  
  Traintastic verwendet keine Consists der Zentrale. Jede Lok wird individuell gesteuert, die Geschwindigkeit wird entsprechend ihres Geschwindigkeitsprofils angepasst.

- **Minimale Slot-Verwaltung**  
  Slot-bezogene Nachrichten werden nur dort unterstützt, wo sie erforderlich sind. Slot-Verknüpfung, Verschieben oder Slot-Juggling wird derzeit nicht unterstützt.

- **Erweiterte Rückmeldung**  
  Zusätzlich zur offiziellen Spezifikation unterstützt Traintastic weitere Nachrichten, die durch Traffic-Analyse identifiziert wurden (z. B. RailCom- und Uhlenbrock-LISSY-Rückmeldungen).

- **Fast-Clock-Unterstützung**  
  Traintastic unterstützt die LocoNet-Fast-Clock in zwei Betriebsarten:
  - **Master-Modus**: Traintastic steuert die Modellzeit und verteilt sie im Bus
  - **Slave-Modus**: Traintastic synchronisiert sich mit einer externen Fast-Clock

- **Reverse Engineering Erweiterungen**  
  Nachrichten, die nicht in der offiziellen *LocoNet Personal Use Edition 1.0 Specification* enthalten sind, werden basierend auf Traffic-Analyse und realem Verhalten implementiert.
```

---

## Message support

### Power control
- `OPC_GPON` – Supported  
- `OPC_GPOFF` – Supported  
- `OPC_IDLE` – Supported  

---

### Locomotive control
- `OPC_LOCO_SPD` – Supported  
- `OPC_LOCO_DIRF` – Supported  
- `OPC_LOCO_SND` – Supported  
- `OPC_IMM_PACKET` (F9–F28) – Supported  
- Uhlenbrock extended functions F9–F28 – Supported (reverse engineered)  

- `OPC_MOVE_SLOTS` – Not yet supported  

- Consists:
  - `OPC_CONSIST_FUNC`
  - `OPC_LINK_SLOTS`
  - `OPC_UNLINK_SLOTS`  
  → Not supported (locomotives are managed individually)

---

### Turnouts, signals, outputs
- `OPC_SW_REQ` – Supported  
- `OPC_SW_REP` – Not supported  
- `OPC_SW_STATE` – Not supported  
- `OPC_SW_ACK` – Not supported  

---

### Feedback sensors
- `OPC_INPUT_REP` – Supported  
- RailCom:
  - `OPC_MULTI_SENSE`
  - `OPC_MULTI_SENSE_LONG`  
  → Supported (reverse engineered)

- Uhlenbrock LISSY (address, category, direction, speed) – Supported (reverse engineered)

---

### Slot and system data
- `OPC_RQ_SL_DATA` – Supported  
- `OPC_SL_RD_DATA` – Supported  
- `OPC_WR_SL_DATA` – Supported (fast clock)  
- `OPC_SLOT_STAT1` – Not supported  
- `OPC_MOVE_SLOTS` – Planned  

---

### Programming
- Decoder programming – Planned  
- `OPC_PEER_XFER` (SV programming) – Planned  1111111111
- Uhlenbrock LNCV programming – Supported (reverse engineered)  

---

## Debugging and monitoring

Traintastic provides a debug mode for LocoNet that logs all bus traffic.

Messages are displayed in hexadecimal format, and many message types include a human-readable interpretation.

This is useful for:

- diagnosing compatibility issues  
- verifying message transmission and reception  
- analyzing vendor-specific or undocumented extensions  

---

## Sending raw messages

Through [Lua scripting](../advanced/scripting-basics.md), it is possible to:

- send raw LocoNet messages via [`send()`](lua/object/loconetinterface.md#send)
- send raw DCC packets via [`imm_packet()`](lua/object/loconetinterface.md#imm_packet)

---

!!! warning
These functions bypass Traintastic’s internal logic.

- requires solid understanding of LocoNet/DCC  
- may cause side effects not tracked by Traintastic  
- misuse can lead to inconsistent system state  
```
