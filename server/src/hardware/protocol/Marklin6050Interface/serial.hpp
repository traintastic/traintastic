/**
 * hardware/protocol/Marklin6050Interface/serial.hpp
 *
 * Cross-platform serial port abstraction for Marklin6050Interface
 * Copyright (C) 2025
 */

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLIN6050INTERFACE_SERIAL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLIN6050INTERFACE_SERIAL_HPP

#include <string>
#include <vector>

class serial
{
public:
    // Return a list of available serial ports on the OS
    static std::vector<std::string> getPortList();

    // Test if a port string is valid
    static bool isValidPort(const std::string& port);

    // Test if the port can be opened
    static bool testOpen(const std::string& port);
};

#endif
