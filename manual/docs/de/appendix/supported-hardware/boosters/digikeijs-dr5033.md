# Digikeijs DR5033

Der **Digikeijs DR5033** ist ein Smart-LocoNet-Booster mit grundlegender Diagnose-Rückmeldung und Unterstützung für Fernsteuerung.

## Unterstützte Diagnosen

- Last
- Temperatur *(wird gemeldet, ist jedoch nicht funktional)*

## Steuerungsmöglichkeiten

- Stromkreis kann über eine Zubehöradresse ein- und ausgeschaltet werden

## Anforderungen und Hinweise

- Der **Temperaturwert wird immer als 0 gemeldet**, obwohl der Booster dieses Feld bereitstellt
- Der **Lastwert bleibt ebenfalls 0**, bis der Gleisstrom ungefähr **400–500 mA** erreicht
- Die Statusrückmeldungen sind im Vergleich zu neueren Smart-Boostern eingeschränkt

## Konfiguration

- **Schnittstelle** – Die verwendete LocoNet-Schnittstelle zur Kommunikation mit dem Booster
- **Adresse** – LNCV-Adresse des Boosters (Werkseinstellung: **1**)
- **Abfrageintervall** – Intervall (in Sekunden), in dem Diagnosedaten abgefragt werden (Standard: **5 Sekunden**)

!!! warning
    Ein **sehr kurzes Abfrageintervall** erhöht den Datenverkehr auf dem LocoNet-Bus und kann die Gesamtleistung des Systems negativ beeinflussen.
    Der Standardwert wird empfohlen, sofern keine schnelleren Aktualisierungen wirklich erforderlich sind.
