# Command line options

The client and server both support various command line options to contol how the applications start and where data is stored.
These options are mainly useful for PCs only used to control the layout.

## Traintastic client

| Short                  | Long                          | Description                  |
|------------------------|-------------------------------|------------------------------|
| `-h`                   | `--help`                      | Displays help text           |
| `-v`                   | `--version`                   | Displays version information |
|                        | `--fullscreen`                | Start application fullscreen |
| `-c <hostname[:port]>` | `--connect <hostname[:port]>` | Connect to server            |

## Traintastic server

Command line options available for Traintastic server depend on the used operation system.
The **operation system** column shows whether an option is available on **All**, **Windows**, **Linux** and/or **macOS**.

| Short     | Long             | Description                              | Operation system |
|-----------|------------------|------------------------------------------|------------------|
| `-h`      | `--help`         | Display help text and exit               | All              |
| `-v`      | `--version`      | Output version information and exit      | All              |
| `-D PATH` | `--datadir PATH` | Data directory                           | All              |
| `-W UUID` | `--world UUID`   | World UUID to load                       | All              |
|           | `--simulate`     | Enable simulation after loading world    | All              |
|           | `--online`       | Enable communication after loading world | All              |
|           | `--power`        | Enable power after loading world         | All              |
|           | `--run`          | Start after loading world                | All              |
|           | `--tray`         | Run application in system tray           | Windows          |
| `-d`      | `--daemonize`    | Run as background daemon                 | Linux, macOS     |
| `-u NAME` | `--user NAME`    | Run as user                              | Linux, macOS     |
| `-g NAME` | `--group NAME`   | Run as group                             | Linux, macOS     |
| `-p [FILENAME]`  | `--pidfile [FILENAME]`  | Write pid file (default: `/run/traintastic-server.pid`) | Linux, macOS |


!!! note
    `--simulate`, `--online`, `--power` and `--run` options only apply to the world loaded at startup.

!!! note
    `--run` option requires `--power`, `--power` option must be set for `--run` to work.

### Data directory

The *data directory* is the location where Traintastic server stores all its data, such as: settings, worlds, logfile, backups.
The default location differs per operating system:

- **Windows**: `%LOCALAPPDATA%\traintastic\server`, e.g. `C:\Users\reinder\AppData\Local\traintastic\server`
- **Linux**:
    - When running as normal user: `~/.config/traintastic-server`, e.g. `/home/reinder/.config/traintastic-server`
    - When running as systemd service: `/var/opt/traintastic`

!!! note
    Traintastic server stores its data per user. To use the same settings, worlds, logfile, backups
    with multiple user accounts they all must start Traintastic server with the *data directory* option pointing to a location that is writable by all involved user accounts.
