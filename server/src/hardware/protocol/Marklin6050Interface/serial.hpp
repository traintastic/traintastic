/**
 * server/src/hardware/serial/Serial.hpp
 *
 * Cross-platform serial port abstraction
 * Supports Windows, macOS, Linux, and Raspbian
 */

#pragma once

#include <string>
#include <vector>

class Serial
{
public:
    // Return a list of available serial port names on the current OS
    static std::vector<std::string> getPortList();

    // Check if a given port name is valid/exists
    static bool isValidPort(const std::string& port);

    // Attempt to open the port for testing; returns true if successful
    static bool testOpen(const std::string& port);

    // Prevent instantiation
    Serial() = delete;
};
