# Common interface connection errors

When Traintastic fails to communicate with a command station or interface, it will report an error that helps identify the underlying connection problem.
This page lists the most common interface-related errors along with their typical causes and steps you can take to resolve them.

## E2005: Socket connect failed (No route to host)

This error indicates that Traintastic tried to open a network connection to the command station,
but your computer could not find a valid network path to the device.
In most cases, this happens when the hostname or IP address is incorrect, the device is powered off, or it is not reachable on the current network.

Please check that the hostname/IP address is correct and that the command station is properly connected to the network.

## E2005: Socket connect failed (Connection refused)

This error means Traintastic reached the device on the network, but the command station rejected the connection.
Common causes include the command station not running its network service yet, the wrong port number being used, or the device being busy or misconfigured.

Please check that the hostname/IP address and port are correct and that the command station is powered on and connected to the network.

## E2010: Serial port open failed (No such file or directory)

This error indicates that Traintastic attempted to open a serial port that does not exist on the system.
This can occur if the device name is typed incorrectly, the command station is not connected, or the operating system has assigned a different port name than expected.

Please check that the selected serial port name is correct and that the command station is properly connected to the computer.

## E2010: Serial port open failed (Permission denied)

This error means Traintastic can see the serial port, but the operating system is preventing access.
This typically occurs when the current user does not have the required permissions for serial devices or when another application is already using the port.

Please check that your user account has permission to access the serial port and that no other application is using the port.

!!! note "Linux"
    Access to serial ports typically requires the user to be added to the **plugdev** group.
