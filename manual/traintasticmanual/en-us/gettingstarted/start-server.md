# Starting the server {#start-server}

When running Traintastic, the server should be started first.

## Windows
The Traintastic server can be started using the desktop icon (if installed) or by selecting *Traintastic* -> *Traintastic server* from the Windows start menu.
Traintastic server runs as background process, a Traintastic icon will appear in the system tray next to the clock.
A Windows notification is displayed when it is running in the background.
Traintastic server can be quit by clicking on the Traintastic icon and selecting *Quit* from the popup menu.

![](../../gfx/en-us/start/start-traintastic-server-windows.png "Traintastic server tray icon and notification")

## Linux
When installing Traintastic server using a Debian package it is installed as systemd service.

To start the Traintastic server using systemd, open a terminal and run the command:
```bash
sudo systemctl start traintastic-server.service
```

To stop the service, use the command:
```bash
sudo systemctl stop traintastic-server.service
```

Ensure you have the necessary permissions (typically root) to manage systemd services.

### Auto start on system boot
To enable Traintastic server to start automatically at boot, run the following command in a terminal:
```bash
sudo systemctl enable traintastic-server.service
```

To disable automatic start at boot, run the following command in a terminal:
```bash
sudo systemctl disable traintastic-server.service
```
