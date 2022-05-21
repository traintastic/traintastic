# Starting the client {#start-client}

When Traintastic server is running the Traintastic client can be started.

**Windows:** The Traintastic client can be started using the desktop icon (if installed) or by selecting *Traintastic* -> *Traintastic client* from the Windows start menu.

**Linux:** TODO

## Connect to the server

When the Traintastic client is started a *connect to server* dialog will appear.
The Traintastic client will automatically search the local network for running Traintastic servers.
Usually there is only one Traintastic server running, connect to it by clicking on the *Connect* button.
When successful the dialog will disappear and the main application dialog becomes active.

## Main menu

TODO

### File

TODO

### View

TODO

### World

TODO

### Objects

TODO

### Tools

TODO

### Help

TODO

## Toolbar

The toolbar contains items that are frequently used when operating the layout. The toolbar can be hidden by selecting *View* -> *Toolbar* in the mainmenu.

### Online/offline

Toggle world online/offline, this sends a go online or go offline event to all command stations.

### Power on/off

Toggle world power, this sends a power on or power off event to all command stations.

### ![](../../gfx/toolbar/stop.png) Stop

Emergency stop all trains.

### ![](../../gfx/toolbar/run.png) Run

Restore last known speed and direction of all trains.

### ![](../../gfx/toolbar/mute.png) Mute

Disable all sound functions, this requires that sound or mute decoder functions are set to the *Sound* or *Mute* type, see [decoder function function](../object/decoderfunction.md#decoder-function-function).

### ![](../../gfx/toolbar/no_smoke.png) No smoke

Disable all smoke generators, this requires that all smoke generator decoder functions are set to the *Smoke* type, see [decoder function function](../object/decoderfunction.md#decoder-function-function).

### Edit mode

Traintastic as a special *edit mode*, when the world is not in edit mode most settings that define the layout can't be changed.
This prevents making accidental changes during layout operation. Changing some settings also also requires the world to be stopped.
