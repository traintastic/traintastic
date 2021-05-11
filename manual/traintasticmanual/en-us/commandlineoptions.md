# Command line options {#command-line-options}

The client and server both support various command line options to contol how the applications start and where data is stored.
These options are mainly useful for PCs only used to control the layout.

## Client {#command-line-options-client}

| Short                  | Long                          | Description                   |
| ---------------------- | ----------------------------- | ----------------------------- |
| `-h`                   | `--help`                      | Displays help text.           |
| `-v`                   | `--version`                   | Displays version information. |
|                        | `--fullscreen`                | Start application fullscreen. |
| `-c <hostname[:port]>` | `--connect <hostname[:port]>` | Connect to server.            |

## Server {#command-line-options-server}

| Short      | Long             | Description                         |
| ---------- | ---------------- | ----------------------------------- |
|  `-h`      | `--help`         | Display help text and exit          |
|  `-v`      | `--version`      | Output version information and exit |
|  `-D path` | `--datadir path` | Data directory                      |
|            | `--tray`         | Run application in system tray (windows only) |
