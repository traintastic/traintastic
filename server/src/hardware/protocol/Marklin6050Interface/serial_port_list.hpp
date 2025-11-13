//6050-6023-interface/server/src/hardware/protocol/Marklin6050Interface/serial.cpp
#pragma once
#include <vector>
#include <string>

#if defined(_WIN32)
#include <windows.h>
inline std::vector<std::string> listSerialPorts() {
    std::vector<std::string> ports;
    char portName[16];
    for (int i = 1; i <= 256; ++i) {
        sprintf(portName, "\\\\.\\COM%d", i);
        HANDLE hPort = CreateFileA(portName, GENERIC_READ | GENERIC_WRITE,
                                   0, NULL, OPEN_EXISTING, 0, NULL);
        if (hPort != INVALID_HANDLE_VALUE) {
            ports.push_back("COM" + std::to_string(i));
            CloseHandle(hPort);
        }
    }
    return ports;
}

#elif defined(__linux__) || defined(__APPLE__)
#include <glob.h>
inline std::vector<std::string> listSerialPorts() {
    std::vector<std::string> ports;
    glob_t glob_result{};
#if defined(__APPLE__)
    glob("/dev/tty.*", 0, nullptr, &glob_result);
#else
    glob("/dev/ttyS*", 0, nullptr, &glob_result);
    glob("/dev/ttyUSB*", GLOB_APPEND, nullptr, &glob_result);
    glob("/dev/ttyACM*", GLOB_APPEND, nullptr, &glob_result);
#endif
    for (size_t i = 0; i < glob_result.gl_pathc; ++i)
        ports.emplace_back(glob_result.gl_pathv[i]);
    globfree(&glob_result);
    return ports;
}
#endif
