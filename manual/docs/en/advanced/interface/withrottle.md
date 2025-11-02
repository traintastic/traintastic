# WiThrottle interface configuration

This page describes how to configure the WiThrottle server in Traintastic.

!!! note
    There is **no setup wizard support** for WiThrottle.
    Configuration must be done manually.

## Overview

WiThrottle is a protocol used by many wireless throttle apps, such as **WiThrottle (iOS)** and **EngineDriver (Android)**.
When enabled, Traintastic acts as a **WiThrottle server** that wireless throttles connect to over the network.

## Connection settings

- **Port** – TCP port that throttles connect to.<br>
  Default: `4444`.<br>
  Normally this does not need to be changed unless the port is already in use.

## WiThrottle settings

### Debugging

- **Debug log RX/TX** – Log all WiThrottle traffic (hex + description) for debugging purposes.

!!! tip "Need help or having issues?"
    If you encounter problems connecting a throttle to Traintastic, check the [community forum](https://discourse.traintastic.org).
    Sharing your experience helps others and improves Traintastic.
