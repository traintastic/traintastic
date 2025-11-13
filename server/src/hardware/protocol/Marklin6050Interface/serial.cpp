//6050-6023-interface/server/src/hardware/protocol/Marklin6050Interface/serial.cpp

#include "serial.hpp"
#include <string>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__) || defined(__linux__)
#include <filesystem>
#include <fcntl.h>    // for O_RDWR, O_NOCTTY, O_NDELAY
#include <unistd.h>   // for close()
#endif

namespace Marklin6050 {
namespace Serial {

std::vector<std::string> listAvailablePorts() {
    std::vector<std::string> ports;

#if defined(_WIN32)
    // Windows COM ports: COM1 ... COM256
    for (int i = 1; i <= 256; ++i) {
        std::string port = "COM" + std::to_string(i);
        HANDLE h = CreateFileA(("\\\\.\\" + port).c_str(),
                               GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                               OPEN_EXISTING, 0, nullptr);
        if (h != INVALID_HANDLE_VALUE) {
            ports.push_back(port);
            CloseHandle(h);
        }
    }
#elif defined(__APPLE__) || defined(__linux__)
    // macOS & Linux typical device paths
    for (const auto& entry : std::filesystem::directory_iterator("/dev")) {
        std::string name = entry.path().string();
        if (name.find("ttyUSB") != std::string::npos ||
            name.find("ttyACM") != std::string::npos ||
            name.find("ttyS")   != std::string::npos ||
            name.find("cu.")    != std::string::npos) {
            ports.push_back(name);
        }
    }
#endif
    return ports;
}

bool isValidPort(const std::string& port) {
    for (auto& p : listAvailablePorts()) {
        if (p == port)
            return true;
    }
    return false;
}

bool testOpen(const std::string& port) {
#if defined(_WIN32)
    HANDLE h = CreateFileA(("\\\\.\\" + port).c_str(),
                           GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                           OPEN_EXISTING, 0, nullptr);
    if (h == INVALID_HANDLE_VALUE)
        return false;
    CloseHandle(h);
    return true;
#else
    int fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0)
        return false;
    close(fd);
    return true;
#endif
}

} // namespace Serial
} // namespace Marklin6050

