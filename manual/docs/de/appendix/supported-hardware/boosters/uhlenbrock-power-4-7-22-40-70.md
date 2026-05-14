# Uhlenbrock Power 4/7/22/40/70

Die **Uhlenbrock Power**-Serie besteht aus intelligenten LocoNet-Boostern, die grundlegende Diagnosefunktionen sowie Fernsteuerung unterstützen.

## Unterstützte Diagnosen

- Last
- Temperatur

## Steuerungsmöglichkeiten

- Stromkreis kann über eine Zubehöradresse ein- und ausgeschaltet werden

## Anforderungen und Hinweise

- Diese Booster wurden mit Traintastic **noch nicht vollständig verifiziert**.
  Aufgrund der Protokollkompatibilität und Dokumentation wird jedoch erwartet, dass sie funktionieren.

## Konfiguration

- **Schnittstelle** – Die verwendete LocoNet-Schnittstelle zur Kommunikation mit dem Booster
- **Adresse** – LNCV-Adresse des Boosters (Werkseinstellung: **1**)
- **Abfrageintervall** – Intervall (in Sekunden), in dem Diagnosedaten abgefragt werden (Standard: **5 Sekunden**)

!!! warning
    Ein **sehr kurzes Abfrageintervall** erhöht den Datenverkehr auf dem LocoNet-Bus und kann die Gesamtleistung des Systems beeinträchtigen.
    Der Standardwert wird empfohlen, sofern keine schnelleren Aktualisierungen erforderlich sind.
