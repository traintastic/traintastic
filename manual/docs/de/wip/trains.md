# Züge

In Traintastic können **nur Züge** direkt gesteuert werden. Züge bestehen aus einem oder mehreren Fahrzeugen, und das Verständnis der Beziehung zwischen Zügen und Fahrzeugen ist entscheidend für das Einrichten und Betreiben eines Layouts.

## Was ist ein Zug?

Ein **Zug** in Traintastic ist eine logische Gruppierung von Fahrzeugen, die gemeinsam bewegt werden. Ein Zug kann enthalten:

- **Angetriebene Fahrzeuge** wie Lokomotiven oder Triebwagen.
- **Nicht angetriebene Fahrzeuge** wie Wagen oder Reisezugwagen.

Züge sind **nicht statisch**: Sie können dynamisch erstellt, verändert, aktiviert, deaktiviert und wiederverwendet werden.

## Fahrzeuge vs. Züge

Fahrzeuge werden im System **unabhängig** von Zügen verwaltet. Das bedeutet:

- Fahrzeuge werden separat im System definiert.
- Ein **einzelnes Fahrzeug** kann Teil von **mehreren Zugdefinitionen** sein.
- Ein Zug ist im Wesentlichen eine Liste von Fahrzeugreferenzen, kein Container, der die Fahrzeuge besitzt.

Diese Trennung ermöglicht Flexibilität, zum Beispiel das Erstellen alternativer Zugzusammenstellungen mit derselben Lokomotive oder denselben Wagen.

## Angetriebene Fahrzeuge

Ein Zug kann **ein oder mehrere angetriebene Fahrzeuge** enthalten. Damit ein Zug jedoch aktiviert und gesteuert werden kann, wird **mindestens ein angetriebenes Fahrzeug** benötigt.

Züge ohne Antrieb (z. B. nur Wagen) **können nicht aktiviert werden** und bleiben im System inaktiv.

## Zugaktivierung

Bevor ein Zug gesteuert werden kann, muss er **aktiviert** werden. Die Aktivierung stellt sicher, dass:

- keines seiner Fahrzeuge bereits Teil eines anderen **aktiven Zuges** ist.
- der Zug mindestens ein **angetriebenes Fahrzeug** enthält.

Ein Fahrzeug kann jeweils nur zu **einem aktiven Zug gleichzeitig** gehören. Der Versuch, einen Zug zu aktivieren, der ein bereits verwendetes Fahrzeug enthält, führt zu einem Fehler.

## Fahrsteuerung (Throttle Control)

Um einen Zug zu steuern, muss er von einem **Fahrregler (Throttle)** übernommen werden. Ein Fahrregler bietet die Schnittstelle für:

- Geschwindigkeit und Fahrtrichtung setzen
- Notstopps auslösen
- Zugstatus überwachen

Wichtige Regeln zur Fahrsteuerung:

- Ein Zug kann zu jedem Zeitpunkt **null oder genau einen aktiven Fahrregler** haben.
- Ein Fahrregler kann die Kontrolle von einem anderen Fahrregler **übernehmen**.
- Die Fahrreglersteuerung ist für manuelle oder skriptbasierte Bedienung erforderlich.

## Arten von Fahrreglern

Traintastic unterstützt verschiedene Arten von Fahrreglern, die sowohl manuelle als auch automatisierte Steuerung ermöglichen:

- **Client-Fahrregler**: Der primäre Fahrregler in der Traintastic-Client-Anwendung.
- **Web-Fahrregler**: Eine webbasierte Oberfläche, die über Browser erreichbar ist.
- **Hardware-Fahrregler**: Externe Geräte, z. B. über das **WiThrottle-Protokoll**.
- **Lua-Skript-Fahrregler**: Ein programmierbarer Fahrregler für Automatisierung und Logik mittels Lua-Skripten.

Diese verschiedenen Fahrreglertypen können parallel existieren, und der passende Typ kann je nach Steuerungsstil oder Automatisierungsanforderung gewählt werden.

