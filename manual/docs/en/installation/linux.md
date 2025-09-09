# Linux Installation

Traintastic provides Debian packages for **Ubuntu (amd64/arm64)** and for **Raspberry Pi (armel/arm64)**.
There are three packages available:

- **traintastic-data** – required data package (must always be installed)
- **traintastic-server** – the main server, required if this machine will control your layout
- **traintastic-client** – graphical client application, optional for headless setups

## Installing

1. Download the `.deb` packages from [traintastic.org/download](https://traintastic.org/download).
2. Install the required packages using `apt` (preferred) or `dpkg`. For example:

```bash
   sudo apt update
   sudo apt install ./traintastic-data_<version>_all.deb \
                    ./traintastic-server_<version>_<arch>.deb \
                    ./traintastic-client_<version>_<arch>.deb
```

Replace `<version>` with the release number (e.g. `0.3.0`) and `<arch>` with your platform (`amd64`, `arm64`, `armhf`).

## Choosing packages

- If this computer will control your layout → install `traintastic-server` + `traintastic-data`.
- If this computer will run the graphical interface only → install `traintastic-client` + `traintastic-data`.
- For most desktop/laptop setups → install all three (`traintastic-server` + `traintastic-client` + `traintastic-data`).

## Running the server (systemd)

When installed via the Debian package, Traintastic server is set up as a systemd service, but it is disabled by default.

Start the server:
```bash
sudo systemctl start traintastic-server.service
```

Stop the server:
```bash
sudo systemctl stop traintastic-server.service
```

You need sufficient permissions (typically root) to manage systemd services.

### Autostart server on Boot

Enable the server to run automatically at system boot:
```bash
sudo systemctl enable traintastic-server.service
```

Disable automatic start:
```bash
sudo systemctl disable traintastic-server.service
```

## Running the client

If you installed the client package, you can start it from your desktop environment’s application launcher menu (look for Traintastic).

---

After installation, continue with: [The Quick Start series](../quickstart/index.md).
