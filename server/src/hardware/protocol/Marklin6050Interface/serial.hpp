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
