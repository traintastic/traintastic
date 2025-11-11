/**
 * hardware/protocol/Marklin6050Interface/serial.hpp
 *
 * Cross-platform serial port helper for Märklin 6050 interface
 * © 2025
 */

#pragma once

#include <string>
#include <vector>

namespace Marklin6050
{
namespace Serial
{
    /**
     * Get a list of available serial ports on the current OS
     */
    std::vector<std::string> getPortList();

    /**
     * Quick check if a port name is valid
     */
    bool isValidPort(const std::string& port);

    /**
     * Test opening the port without fully connecting
     */
    bool testOpen(const std::string& port);
}
}
