#include "serial.hpp"
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace Marklin6050
{
namespace Serial
{

std::vector<std::string> getPortList()
{
    std::vector<std::string> ports;

#ifdef _WIN32
    // Check COM1..COM256
    for(int i=1;i<=256;i++)
    {
        std::string name = "COM" + std::to_string(i);
        HANDLE h = CreateFileA(name.c_str(), GENERIC_READ|GENERIC_WRITE,
                               0, nullptr, OPEN_EXISTING, 0, nullptr);
        if(h != INVALID_HANDLE_VALUE)
            CloseHandle(h), ports.push_back(name);
    }
#elif __APPLE__
    // macOS typical tty devices
    const char* devs[] = {"/dev/tty.usbserial", "/dev/tty.usbmodem"};
    for(auto dev : devs)
        ports.push_back(dev);
#else
    // Linux /dev/ttyS*, /dev/ttyUSB*, /dev/ttyAMA*, /dev/serial/by-id/*
    const char* prefixes[] = {"/dev/ttyS", "/dev/ttyUSB", "/dev/ttyAMA", "/dev/serial/by-id/"};
    for(auto prefix : prefixes)
    {
        for(int i=0;i<256;i++)
        {
            std::string port = std::string(prefix) + std::to_string(i);
            if(access(port.c_str(), F_OK) == 0)
                ports.push_back(port);
        }
    }
#endif

    std::sort(ports.begin(), ports.end());
    return ports;
}

bool isValidPort(const std::string& port)
{
    auto ports = getPortList();
    return std::find(ports.begin(), ports.end(), port) != ports.end();
}

bool testOpen(const std::string& port)
{
#ifdef _WIN32
    HANDLE h = CreateFileA(port.c_str(), GENERIC_READ|GENERIC_WRITE,
                           0, nullptr, OPEN_EXISTING, 0, nullptr);
    if(h == INVALID_HANDLE_VALUE)
        return false;
    CloseHandle(h);
    return true;
#else
    int fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if(fd < 0) return false;
    close(fd);
    return true;
#endif
}

}
}

}
}
