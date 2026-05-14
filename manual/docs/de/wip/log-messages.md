# Nachrichten

Jede Log-Nachricht hat einen eindeutigen Code. Dieser beginnt mit einem Buchstaben, der die Schwere (Level) angibt:

| Buchstabe | Level                 | Beschreibung                                         |
|-----------|----------------------|------------------------------------------------------|
| **F**     | [Fatal](#fatal)      |                                                      |
| **C**     | [Critical](#critical)| Kritische Nachrichten zeigen an, dass Handeln nötig ist. |
| **E**     | [Error](#error)      |                                                      |
| **W**     | [Warning](#warning)  |                                                      |
| **N**     | [Notice](#notice)    |                                                      |
| **I**     | [Info](#info)        |                                                      |
| **D**     | [Debug](#debug)      |                                                      |

Dazu kommt eine vierstellige Nummer, die die Nachricht identifiziert. Der Zahlenbereich ist in Kategorien unterteilt, um die Quelle der Nachricht schneller zu erkennen:

| Nummern      | Kategorie                    |
|--------------|-----------------------------|
| 1000 … 1999  | Traintastic Kernkomponenten |
| 2000 … 2999  | Hardware-Anbindung         |
| 3000 … 3999  | Züge und Schienenfahrzeuge |
| 4000 … 8999  | *unbenutzt*                |
| 9000 … 9999  | Lua-Skripting             |

## Fatal {#fatal}

TODO


### F1001: Öffnen des TCP-Sockets fehlgeschlagen (*reason*) {#f1001}

TODO


### F1002: TCP-Socket Address-Reuse fehlgeschlagen (*reason*) {#f1002}

TODO


### F1003: Binden des TCP-Sockets fehlgeschlagen (*reason*) {#f1003}

TODO


### F1004: TCP-Socket Listen fehlgeschlagen (*reason*) {#f1004}

TODO


### F1005: Öffnen des UDP-Sockets fehlgeschlagen (*reason*) {#f1005}

TODO


### F1006: UDP-Socket Address-Reuse fehlgeschlagen (*reason*) {#f1006}

TODO


### F1007: Binden des UDP-Sockets fehlgeschlagen (*reason*) {#f1007}

TODO


### F9001: Erstellen des Lua-States fehlgeschlagen {#f9001}

TODO


### F9002: Ausführen des Skripts fehlgeschlagen (*reason*) {#f9002}

TODO


### F9003: Aufruf der Funktion fehlgeschlagen (*reason*) {#f9003}

TODO


### F9999: *message* {#f9999}

Benutzerdefinierte Fatal-Nachricht, erzeugt durch ein [Lua-Skript](../advanced/scripting-basics.md).


## Critical {#critical}

Kritische Nachrichten zeigen an, dass Handeln erforderlich ist.


### C1001: Laden der Welt fehlgeschlagen (*reason*) {#c1001}

TODO


### C1002: Erstellen des Clients fehlgeschlagen (*reason*) {#c1002}

TODO


### C1003: Schreiben der Einstellungsdatei nicht möglich (*reason*) {#c1003}

TODO


### C1004: Lesen der Welt fehlgeschlagen (*reason*) (*filename*) {#c1004}

TODO


### C1005: Speichern der Welt fehlgeschlagen (*reason*) {#c1005}

TODO


### C1006: Erstellen des Welt-Backups fehlgeschlagen (*reason*) {#c1006}

TODO


### C1007: Erstellen des Backup-Verzeichnisses fehlgeschlagen (*reason*) {#c1007}

TODO

### C1008: Erstellen des Backup-Verzeichnisses fehlgeschlagen (*reason*) {#c1008}
TODO

### C1009: Erstellen des Einstellungs-Backups fehlgeschlagen (*reason*) {#c1009}
TODO

### C1010: Export der Welt fehlgeschlagen (*reason*) {#c1010}
TODO

### C1011: Import der Welt fehlgeschlagen (*reason*) {#c1011}
TODO

### C1012: Unbekannte Klasse '*class id*', Objekt '*object id*' kann nicht wiederhergestellt werden {#c1012}
TODO

### C1013: Welt wurde mit neuerer Version gespeichert, mindestens erforderlich: Traintastic Server *version* {#c1013}

**Ursache:** Die Welt wurde mit einer neueren Version des Traintastic-Servers gespeichert als der aktuell laufenden.

**Lösung:** Traintastic Server (und Client) auf mindestens Version *version* aktualisieren.


### C2001: Adresse bereits in Verwendung bei #*object* {#c2001}

TODO


### C2002: DCC++ unterstützt nur das DCC-Protokoll {#c2002}

**Ursache:** Das gewählte Decoder-Protokoll wird vom DCC++ Command Station nicht unterstützt.

**Lösung:** Decoder-Protokoll auf *DCC* oder *Auto* ändern.


### C2003: DCC++ unterstützt keine DCC-Long-Addresses unter 128 {#c2003}

**Ursache:** Die DCC++ Command Station behandelt alle Adressen unter 128 als DCC-Kurzadresse.

**Lösung:** Decoder-Adresse ändern.


### C2004: Kein freier Slot verfügbar {#c2004}
TODO

### C9999: *message* {#c9999}

Benutzerdefinierte Critical-Nachricht, erzeugt durch ein [Lua-Skript](../advanced/scripting-basics.md).


## Error {#error}

TODO


### E1001: Ungültige World-UUID: *uuid* {#e1001}

TODO


### E1002: World *uuid* existiert nicht {#e1002}

TODO


### E1003: UDP-Empfangsfehler (*reason*) {#e1003}

TODO


### E1004: TCP-Accept-Fehler (*reason*) {#e1004}

TODO


### E1005: Socket-Shutdown fehlgeschlagen (*reason*) {#e1005}

TODO


### E1006: Socket-Schreibfehler (*reason*) {#e1006}

TODO


### E1007: Socket-Lesefehler (*reason*) {#e1007}

TODO


### E1008: Socket-Acceptor-Abbruch fehlgeschlagen (*reason*) {#e1008}

TODO


### E2001: Serielle Schreiboperation fehlgeschlagen (*reason*) {#e2001}

TODO


### E2002: Serielle Leseoperation fehlgeschlagen (*reason*) {#e2002}

TODO


### E2003: Adress-Erstellung fehlgeschlagen (*reason*) {#e2003}

TODO


### E2004: Socket-Öffnen fehlgeschlagen (*reason*) {#e2004}

TODO


### E2005: Socket-Verbindung fehlgeschlagen (*reason*) {#e2005}

TODO


### E2006: Socket-Bind fehlgeschlagen (*reason*) {#e2006}

TODO


### E2007: Socket-Schreibfehler (*reason*) {#e2007}

TODO


### E2008: Socket-Lesefehler (*reason*) {#e2008}

TODO


### E2009: Socket-Empfang fehlgeschlagen (*reason*) {#e2009}

TODO


### E2010: Serial-Port öffnen fehlgeschlagen (*reason*) {#e2010}

TODO


### E2011: Socket-Senden fehlgeschlagen (*reason*) {#e2011}

TODO


### E2012: Funktionsnummer bereits in Verwendung {#e2012}

TODO

### E2013: Serial-Port Baudrate setzen fehlgeschlagen (*reason*) {#e2013}
TODO

### E2014: Serial-Port Datenbits setzen fehlgeschlagen (*reason*) {#e2014}
TODO

### E2015: Serial-Port Stopbits setzen fehlgeschlagen (*reason*) {#e2015}
TODO

### E2016: Serial-Port Parität setzen fehlgeschlagen (*reason*) {#e2016}
TODO

### E2017: Serial-Port Flow-Control setzen fehlgeschlagen (*reason*) {#e2017}
TODO

### E2018: Timeout, kein Echo innerhalb von *number*ms {#e2018}
TODO

### E2019: Timeout, keine Antwort innerhalb von *number*ms {#e2019}
TODO

### E2020: Anzahl der Module darf *number* nicht überschreiten {#e2020}

**Ursache:** Die maximale Anzahl an S88-Modulen am HSI-88 darf nicht überschritten werden. Dies ist eine Hardware-Beschränkung.

**Lösung:** Anzahl der Module reduzieren. Die Summe aus *left* + *middle* + *right* muss ≤ *number* sein.

### E9001: *error* (Während Ausführung des Event-Handlers *name*) {#e9001}
TODO

### E9999: *message* {#e9999}

Benutzerdefinierte Error-Nachricht, erzeugt durch ein [Lua-Skript](../advanced/scripting-basics.md).


## Warning {#warning}

TODO


### W1001: Discovery deaktiviert, nur auf Port *number* erlaubt {#w1001}

TODO


### W1002: Einstellung *name* existiert nicht {#w1002}

TODO

### W1003: Lesen der Welt *filename* fehlgeschlagen (libarchive Fehler *code*: *reason*) {#w1003}
TODO

### W2001: Fehlerhafte Daten empfangen, *number* Bytes verworfen {#w2001}

TODO


### W2002: Command Station unterstützt keine Funktionen über F*number* {#w2002}

**Ursache:** Command Station oder Interface kann diese Funktionen nicht steuern (Hardware-/Protokoll-Limit).

**Lösung:** Konfiguration prüfen oder Funktionen remappen.

### W2003: Command Station unterstützt keine *number* Fahrstufen, verwende *number* {#w2003}

**Ursache:** Begrenzung der Fahrstufen durch Hardware/Interface.

**Lösung:** Decoder auf *Auto* setzen.

### W2004: Eingangsadresse *address* ist ungültig {#w2004}
Das angeschlossene Traintastic DIY Gerät besitzt keinen Eingang mit dieser Adresse.

### W2005: Ausgangsadresse *address* ist ungültig {#w2005}
Das angeschlossene Traintastic DIY Gerät besitzt keinen Ausgang mit dieser Adresse.

### W2006: Command Station unterstützt keinen LocoNet Slot *slot* {#w2006}
Slot kann nicht verwendet werden, LocoNet *Locomotive slots* reduzieren.

### W2007: Command Station unterstützt keinen Fast-Clock-Slot {#w2007}
Fast-Clock kann nicht verwendet werden, LocoNet *Fast clock sync enabled* deaktivieren.

### W9999: *message* {#w9999}

Benutzerdefinierte Warning-Nachricht, erzeugt durch ein [Lua-Skript](../advanced/scripting-basics.md).


## Notice {#notice}

TODO


### N1001: Signal empfangen: *name* {#n1001}

TODO


### N1002: Neue Welt erstellt {#n1002}

TODO


### N1003: Neustart läuft {#n1003}

TODO


### N1004: Herunterfahren läuft {#n1004}

TODO


### N1005: Discovery aktiviert {#n1005}

TODO


### N1006: Discovery deaktiviert {#n1006}

TODO


### N1007: Lausche auf *address*:*port* {#n1007}

TODO


### N1008: Einstellungen geladen {#n1008}

TODO


### N1009: Einstellungen gespeichert {#n1009}

TODO


### N1010: Edit-Modus: aktiviert {#n1010}

TODO


### N1011: Edit-Modus: deaktiviert {#n1011}

TODO


### N1012: Kommunikation: aktiviert {#n1012}

TODO


### N1013: Kommunikation: deaktiviert {#n1013}

TODO


### N1014: Strom: ein {#n1014}

TODO


### N1015: Strom: aus {#n1015}

TODO


### N1016: Läuft {#n1016}

TODO


### N1017: Gestoppt {#n1017}

TODO


### N1018: Mute: aktiviert {#n1018}

TODO


### N1019: Mute: deaktiviert {#n1019}

TODO


### N1020: Rauch: aktiviert {#n1020}

TODO


### N1021: Rauch: deaktiviert {#n1021}

TODO


### N1022: Welt gespeichert: *name* {#n1022}

TODO

### N1023: Simulation: deaktiviert {#n1023}
TODO

### N1024: Simulation: aktiviert {#n1024}
TODO

### N1025: Welt erfolgreich exportiert {#n1025}
TODO

### N1026: Welt erfolgreich importiert {#n1026}
TODO

### N2001: Simulation nicht unterstützt {#n2001}
TODO

### N2002: Keine Antwort vom LNCV-Modul *id* mit Adresse *address* {#n2002}
TODO

### N2003: Fast-Clock-Sync wird nicht mehr gesendet {#n2003}
Eine Fast-Clock-Synchronisation wird nicht mehr zyklisch gesendet, siehe [W2007](#w2007).

### N9001: Skript startet {#n9001}
TODO

### N9999: *message* {#n9999}

Benutzerdefinierte Info-Nachricht, erzeugt durch ein [Lua-Skript](../advanced/scripting-basics.md).


## Info {#info}

Informationsmeldungen


### I1001: Traintastic v*version* *codename* {#i1001}

TODO


### I1002: Einstellungsdatei nicht gefunden, Standardwerte werden verwendet {#i1002}

TODO


### I1003: Client verbunden {#i1003}

TODO


### I1004: Verbindung verloren {#i1004}

TODO


### I1005: Weltindex wird erstellt {#i1005}

TODO

### I1006: *boost version* {#i1006}
Versionsinformation der verwendeten Boost-Bibliothek, z. B. *boost 1.71.0*.

### I1007: *nlohmann::json version* {#i1007}
Versionsinformation der verwendeten nlohmann::json Bibliothek, z. B. *nlohmann::json 3.10.5*.

### I1008: *archive version* {#i1008}
Versionsinformation der verwendeten Archive-Bibliothek, z. B. *libarchive 3.4.0 zlib/1.2.11 liblzma/5.2.4 bz2lib/1.0.8 liblz4/1.9.2 libzstd/1.4.4*.

### I2001: Unbekannte Lok-Adresse: *address* {#i2001}

TODO


### I2002: Hardwaretyp: *type* {#i2002}
TODO


### I2003: Firmware-Version: *version* {#i2003}
TODO


### I2004: HSI-88: *info* {#i2004}

Informationen zur verbundenen HSI-88 Schnittstelle, z. B. *Ver. 0.62 / 08.07.02 / HSI-88 / (c) LDT*.


### I2005: *info* {#i2005}
Informationen zum verbundenen Traintastic DIY Gerät.

### I9001: Skript gestoppt {#i9001}
TODO


### I9002: *lua version* {#i9002}
Versionsinformation der verwendeten Lua-Engine, z. B. *Lua 5.3.3  Copyright (C) 1994-2016 Lua.org, PUC-Rio*.

### I9999: *message* {#i9999}

Benutzerdefinierte Info-Nachricht, erzeugt durch ein [Lua-Skript](../advanced/scripting-basics.md).


## Debug {#debug}

TODO


### D2001: TX: *data* {#d2001}

TODO


### D2002: RX: *data* {#d2002}

TODO


### D2003: Unbekannter xHeader 0x*value* {#d2003}

TODO


### D2004: *source* TX: *data* {#d2004}

TODO


### D2005: *source* RX: *data* {#d2005}

TODO


### D2006: Unbekannte Nachricht: *number* {#d2006}

TODO


### D2007: Eingang *number* = *value* {#d2007}

TODO


### D2008: Ausgang *number* = *value* {#d2008}

TODO


### D2009: Slot *number* = *address* {#d2009}

TODO


### D2010: Slot *number* = frei {#d2010}

TODO


### D9999: *message* {#d9999}

Benutzerdefinierte Debug-Nachricht, erzeugt durch ein [Lua-Skript](../advanced/scripting-basics.md).

