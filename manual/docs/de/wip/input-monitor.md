# [[class:input_monitor]]

[[gfx:inputmonitordialog.png]]

Der Eingangsmonitor kann verwendet werden, um eine Gruppe von Eingängen auf einem bestimmten Bus zu überwachen, z. B. LocoNet oder S88. Mit dem Eingangsmonitor lässt sich leicht testen, ob Eingänge wie erwartet funktionieren, oder Eingänge finden, wenn ihre Adresse nicht bekannt ist.

## Statusanzeige

|| [[gfx:inputmonitorledoff.png]] || Low ||
|| [[gfx:inputmonitorledoon.png]] || High ||
|| [[gfx:inputmonitorledundefined.png]] || Undefined ||

Ein Eingang wird als *undefined* angezeigt, wenn:

- der Eingang auf dem Bus nicht existiert oder
- der Eingang auf dem Bus existiert, aber seinen Status noch nicht gemeldet hat.

[[note:LocoNet-Eingänge melden ihren Status nach dem Einschalten. Das Abschalten und erneute Einschalten der Gleisspannung kann eine Statusmeldung auslösen.]]

## Beschriftung der Anzeige

Weiße Beschriftungen zeigen an, dass der Eingang in der Welt definiert ist. Durch Anklicken kann der Eingangs-Dialog geöffnet werden.

Graue Beschriftungen zeigen an, dass der Eingang noch nicht in der Welt definiert ist. Wenn sich die Welt im Bearbeitungsmodus befindet, kann der Eingang durch Doppelklick erstellt werden.

