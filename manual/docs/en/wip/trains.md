# Trains

In Traintastic, **only trains** can be directly controlled. Trains are composed of one or more vehicles, and understanding how trains and vehicles interact is crucial for setting up and operating a layout effectively.

## What is a Train?

A **train** in Traintastic is a logical grouping of vehicles that move together. A train can include:

- **Powered vehicles** such as locomotives or self-propelled railcars.
- **Unpowered vehicles** such as wagons or coaches.

Trains are **not static**: they can be created, modified, activated, deactivated, and reused dynamically.

## Vehicles vs Trains

Vehicles are managed **independently** from trains. This means:

- Vehicles are defined separately in the system.
- A **single vehicle** can be a part of **multiple train definitions**.
- A train is essentially a list of vehicle references, not a container that owns the vehicles.

This separation allows flexibility, such as creating alternate train consists using the same locomotive or rolling stock.

## Powered Vehicles

A train can include **one or more powered vehicles**. However, in order for a train to be activated and controlled, **at least one powered vehicle** is required.

Unpowered trains (e.g. a consist of only wagons) **cannot be activated** and will remain inactive in the system.

## Train Activation

Before a train can be controlled, it must be **activated**. Activation ensures that:

- None of its vehicles are already part of another **active** train.
- The train has at least one **powered vehicle**.

A vehicle can only belong to **one active train at a time**. Attempting to activate a train that includes a vehicle already in use will result in an error.

## Throttle Control

To control a train, it must be **acquired** by a **throttle**. A throttle provides the interface to:

- Set speed and direction
- Issue emergency stops
- Monitor train state

Important rules around throttle control:

- A train can have **zero or one active throttle** at any given time.
- A throttle can **steal control** from another throttle that currently holds it.
- Throttle control is required for manual or script-based operation.

## Throttle Types

Traintastic supports various throttle types, allowing both manual and automated control:

- **Client throttle**: The primary throttle used in the Traintastic client application.
- **Web throttle**: A web-based interface accessible via browsers.
- **Hardware throttles**: External devices like those using the **WiThrottle** protocol.
- **Lua script throttle**: A programmable throttle controlled by custom Lua scripts for automation and logic.

These different throttle types can coexist, and you can choose the one that best fits your control style or automation requirements.

