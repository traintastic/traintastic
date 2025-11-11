#include "Serial.hpp"

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>
#else
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include <algorithm>
#include <iostream>

std::vector<std::string> Serial::getPortList()
{
    std::vector<std::string> ports;

#if defined(_WIN32)
    // COM1..COM256
    for (int i = 1; i <= 256; ++i)
    {
        std::string port = "COM" + std::to_string(i);
        HANDLE h = CreateFileA(port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
        if (h != INVALID_HANDLE_VALUE)
            CloseHandle(h), ports.push_back(port);
    }
#elif defined(__APPLE__)
    // List /dev/cu.* devices (common for macOS)
    DIR* dir = opendir("/dev");
    if (dir)
    {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr)
        {
            std::string name = entry->d_name;
            if (name.find("cu.") == 0)
                ports.push_back("/dev/" + name);
        }
        closedir(dir);
    }
#else
    // Linux / Raspbian: /dev/ttyS*, /dev/ttyUSB*, /dev/ttyACM*
    const std::vector<std::string> prefixes = { "ttyS", "ttyUSB", "ttyACM" };
    DIR* dir = opendir("/dev");
    if (dir)
    {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr)
        {
            std::string name = entry->d_name;
            for (auto& pre : prefixes)
            {
                if (name.find(pre) == 0)
                {
                    ports.push_back("/dev/" + name);
                    break;
                }
            }
        }
        closedir(dir);
    }
#endif

    std::sort(ports.begin(), ports.end());
    return ports;
}

bool Serial::isValidPort(const std::string& port)
{
    auto ports = getPortList();
    return std::find(ports.begin(), ports.end(), port) != ports.end();
}

bool Serial::testOpen(const std::string& port)
{
#if defined(_WIN32)
    HANDLE h = CreateFileA(port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (h != INVALID_HANDLE_VALUE)
    {
        CloseHandle(h);
        return true;
    }
    return false;
#else
    int fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd >= 0)
    {
        close(fd);
        return true;
    }
    return false;
#endif
}

