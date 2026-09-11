# Häufige Fehler bei Interface-Verbindungen

Wenn Traintastic keine Verbindung zu einer Zentrale oder einem Interface herstellen kann, wird ein Fehler angezeigt, der auf die Ursache hinweist.  
Diese Seite listet die häufigsten Verbindungsfehler und typische Lösungen.

---

## E2005: Socket connect failed (No route to host)

Dieser Fehler bedeutet, dass keine Netzwerkroute zum Zielgerät gefunden werden konnte.  
Traintastic kann die IP-Adresse oder den Hostnamen nicht erreichen.

Typische Ursachen:
- falsche IP-Adresse oder Hostname
- Gerät ist ausgeschaltet
- Gerät ist nicht im gleichen Netzwerk erreichbar
- Routing- oder WLAN-Problem

---

## E2005: Socket connect failed (Connection refused)

Hier wurde das Gerät im Netzwerk erreicht, aber die Verbindung wurde aktiv abgelehnt.  
Das bedeutet, dass der Dienst auf der Gegenseite nicht angenommen hat.

Typische Ursachen:
- falscher Port
- Netzwerkdienst des Command Stations läuft nicht
- Gerät ist noch nicht vollständig gestartet
- falsches Protokoll ausgewählt

---

## E2010: Serial port open failed (No such file or directory)

Traintastic versucht einen seriellen Port zu öffnen, der im System nicht existiert.

Typische Ursachen:
- falscher Gerätepfad (z. B. `/dev/ttyUSB0`)
- USB-Adapter nicht verbunden
- Gerät wurde unter anderem Namen erkannt (z. B. `/dev/ttyACM0`)

---

## E2010: Serial port open failed (Permission denied)

Der serielle Port existiert, aber der Zugriff wird vom Betriebssystem blockiert.

Typische Ursachen:
- fehlende Berechtigungen für serielle Geräte
- Port wird bereits von einer anderen Anwendung genutzt

---

!!! note "Linux"
    Zugriff auf serielle Geräte erfordert in der Regel die Mitgliedschaft in der **plugdev**-Gruppe oder entsprechenden udev-Regeln.
