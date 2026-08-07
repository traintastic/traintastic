# Scripting-Referenz – Einführung

Traintastic integriert eine [Lua](https://www.lua.org)-Skripting-Engine, mit der sich die Modellbahnwelt erweitern und automatisieren lässt.
Diese Referenz beschreibt alle in Traintastic verfügbaren Skriptfunktionen: eingebaute Globals, Bibliotheken, Objekte und Erweiterungen.

## Verwendung dieser Referenz

Diese Referenz ist in folgende Bereiche unterteilt:

- **Globals** – vordefinierte Variablen und Funktionen, die überall verfügbar sind.
- **Bibliotheken** – erweiterte Funktionen wie Mathematik, Zeichenketten und Logging.
- **Persistente Variablen** – Daten, die über Speichern/Laden der Welt hinweg erhalten bleiben.
- **Enums, Sets, Objekte** – Traintastic-spezifische Typen zur Interaktion mit der Welt.
- **Beispiele** – praktische Anwendungsfälle, die die oben genannten Elemente kombinieren.

!!! tip
    Wer bereits mit Lua vertraut ist, kann die Sprachgrundlagen überspringen und direkt zu den [Globals](globals.md) gehen.
    Für Einsteiger empfiehlt sich zuerst die [Lua-Sprachgrundlagen](basics.md).

## Reines Lua vs. Traintastic-Erweiterungen

Die meisten Standardfunktionen von Lua sind verfügbar: Zahlen, Strings, Tabellen, Kontrollstrukturen, Funktionen usw.
Traintastic erweitert Lua um zusätzliche Funktionen zur Steuerung der Simulation:

- **Die `world`-Variable** – Einstiegspunkt zur Interaktion mit der Welt.
- **Enums und Sets** – spezielle Typen zur Abbildung modellbahnspezifischer Konzepte.
- **Objekte** – Live-Referenzen auf Züge, Fahrzeuge, Sensoren, Weichen und mehr.

Zusammen ermöglichen diese Erweiterungen leistungsfähige Automatisierungen – von einfachen Makros bis hin zu komplexer Zugsteuerung.

## Erste Schritte

Zum Ausprobieren von Skripten:

1. Die Liste der **Lua-Skripte** über *Objekte → Lua-Skripte* im Hauptmenü öffnen.
2. Ein neues Skript erstellen und im Editor öffnen.
3. In der Skriptliste **Alle ausführen** drücken, um das Skript auszuführen.

## Nächste Schritte

- Die [Lua-Sprachgrundlagen](basics.md) lesen, falls Lua noch unbekannt ist.
- Die Seite [Globals](globals.md) erkunden, um verfügbare Funktionen zu sehen.
- Die [Beispiele](examples.md) für fertige Code-Snippets ansehen.
