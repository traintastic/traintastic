# Uhlenbrock IB-COM {#uhlenbrock-ibcom}

The Uhlenbrock IB-COM is a LocoNet command station specifically designed to be used for a computer controlled layout.
It can operate up to 32 trains at the same time.

## Interface setup

To use the Uhlenbrock IB-COM with Traintastic a LocoNet interface must be created.
The Uhlenbrock IB-COM must be connected via USB to the computer running the Traintastic server application.

### General settings

| Setting      | Value       |
|--------------|-------------|
| Type         | Serial      |
| Device       | *see below* |
| Baudrate     | 115200      |
| Flow control | Hardware    |

On *Windows* the IB-COM is usually connects as `COM3` or `COM4`, to be sure open the Windows device manager and then connect the USB plug.

On *Linux* the IB-COM is usually connects as `/dev/ttyUSB0`, to be sure run `dmesg` in the console just after connecting the USB plug.

### LocoNet settings

From the command station dropdown select *Uhlenbrock IB-COM*, this will automatically set most LocoNet settings to the best values for the Uhlenbrock IB-COM.

The Uhlenbrock IB-COM does not support LocoNet fast clock, unless there is a fast clock master connected to the LocoNet the *Fast clock* setting should be *Off* and the *Fast clock sync enabled* should be *unchecked*.
