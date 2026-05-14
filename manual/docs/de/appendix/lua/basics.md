# Lua-Sprachgrundlagen

Diese Seite führt in die grundlegende Syntax und die Konzepte der Programmiersprache Lua ein.
Wer bereits mit Lua vertraut ist, kann diese Seite überspringen und direkt mit den [Globals](globals.md) fortfahren.

Lua ist eine leichtgewichtige und einfach zu erlernende Skriptsprache. Sie ist bewusst einfach, flexibel und gut einbettbar.
Traintastic verwendet Lua, um Skripte zu ermöglichen, die mit der Modellbahnwelt interagieren.

## Variablen

Variablen werden verwendet, um Werte zu speichern.

```lua
name = "Traintastic"
speed = 80
enabled = true
````

* Zeichenketten werden in Anführungszeichen geschrieben (`"Text"`).
* Zahlen können Ganzzahlen oder Dezimalzahlen sein.
* Boolesche Werte sind `true` oder `false`.

## Kommentare

Kommentare werden von Lua ignoriert, helfen aber beim Verständnis des Codes.

```lua
-- Das ist ein einzeiliger Kommentar

--[[
Das ist ein
mehrzeiliger Kommentar
]]
```

## Kontrollstrukturen

Lua verwendet klassische Programmierkonstrukte.

### If / else

```lua
if speed > 60 then
  log.info("Schnell!")
else
  log.info("Langsam.")
end
```

### Schleifen

```lua
for i = 1, 5 do
  log.debug("Schritt", i)
end
```

```lua
while enabled do
  log.debug("Läuft...")
  break
end
```

## Funktionen

Funktionen fassen Code in wiederverwendbare Blöcke zusammen.

```lua
function greet(name)
  log.info("Hallo " .. name)
end

greet("Traintastic")
```

## Tabellen

Tabellen sind die einzige Datenstruktur in Lua. Sie funktionieren als Listen, Dictionaries oder Objekte.

```lua
car = { type = "Güterwagen", cargo = "Kohle" }
log.debug(car.type)        -- "Güterwagen"

numbers = { 10, 20, 30 }
log.debug(numbers[1])      -- 10
```

## Alles zusammen

Mit diesen Grundlagen – Variablen, Kontrollstrukturen, Funktionen und Tabellen – lassen sich bereits nützliche Skripte in Traintastic erstellen.

!!! tip
Nicht alles muss sofort gelernt werden. Am besten klein anfangen und Schritt für Schritt erweitern.

## Nächste Schritte

* Erweiterungen von Traintastic in [Globals](globals.md) kennenlernen.
* [Persistente Variablen](pv.md) verwenden, um Daten zwischen Sitzungen zu speichern.
* In den [Beispielen](examples.md) praktische Anwendungen entdecken.

