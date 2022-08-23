# Command line options {#command-line-options}

The client and server both support various command line options to contol how the applications start and where data is stored.
These options are mainly useful for PCs only used to control the layout.

## Traintastic client {#command-line-options-client}

| Short                  | Long                          | Description                   |
|------------------------|-------------------------------|-------------------------------|
| `-h`                   | `--help`                      | Displays help text.           |
| `-v`                   | `--version`                   | Displays version information. |
|                        | `--fullscreen`                | Start application fullscreen. |
| `-c <hostname[:port]>` | `--connect <hostname[:port]>` | Connect to server.            |

## Traintastic server {#command-line-options-server}

Command line options available for Traintastic server depend on the used operation system.


### Generic

Traintastic server command line options available on all supported operating systems:

| Short     | Long             | Description                         |
|-----------|------------------|-------------------------------------|
| `-h`      | `--help`         | Display help text and exit          |
| `-v`      | `--version`      | Output version information and exit |
| `-D PATH` | `--datadir PATH` | Data directory                      |

#### Data directory

The *data directory* is the location where Traintastic server stores all its data, such as: settings, worlds, logfile, backups.
The default location differs per operating system:
- **Windows**: `%LOCALAPPDATA%\traintastic\server`, e.g. `C:\Users\reinder\AppData\Local\traintastic\server`
- **Linux**: `~/.config/traintastic-server`, e.g. `/home/reinder/.config/traintastic-server`

Note: Traintastic server stores its data per user. To use the same settings, worlds, logfile, backups
with multiple user accounts they all must start Traintastic server with the *data directory* option pointing to a location that is writable by all involved user accounts.

### Windows

Traintastic server command line options only available for Windows:

| Short | Long     | Description                    |
|-------|----------|--------------------------------|
|       | `--tray` | Run application in system tray |


### Linux and macOS

Traintastic server command line options only available for Linux and macOS:

| Short           | Long                   | Description                                                          |
|-----------------|------------------------|----------------------------------------------------------------------|
| `-d`            | `--daemonize`          | Daemonize                                                            |
| `-u USERNAME`   | `--user USERNAME`      | Run as user                                                          |
| `-g GROUPNAME`  | `--group GROUPNAME`    | Run as group                                                         |
| `-p [FILENAME]` | `--pidfile [FILENAME]` | Write pid file, `FILENAME` defaults to `/run/traintastic-server.pid` |
