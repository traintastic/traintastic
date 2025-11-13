//6050-6023-interface/server/src/hardware/protocol/Marklin6050Interface/serial.hpp
#pragma once
#include <string>
#include <vector>

namespace Marklin6050 {
namespace Serial {

std::vector<std::string> listAvailablePorts();
bool isValidPort(const std::string& port);
bool testOpen(const std::string& port);

} // namespace Serial
} // namespace Marklin6050
