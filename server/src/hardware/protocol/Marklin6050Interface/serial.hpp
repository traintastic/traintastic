/**
 * server/src/hardware/protocol/Marklin6050Interface/serial.hpp
 *
 * Cross-platform serial port utility for Traintastic interfaces.
 * Detects real serial ports on Windows, Linux, macOS, and Raspbian.
 * Pure C++17 — no external dependencies.
 *
 * © 2025
 */

#ifndef TRAINTASTIC_SERVER_HARDWARE_SERIAL_SERIAL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_SERIAL_SERIAL_HPP

#include <string>
#include <vector>
#include <regex>
#include <filesystem>

#if defined(_WIN32)
  #include <windows.h>
  #include <initguid.h>
  #include <devguid.h>
  #include <setupapi.h>
  #pragma comment(lib, "setupapi.lib")
#endif

namespace Serial
{
  /**
   * \brief Return a list of available serial ports on this system.
   */
  inline std::vector<std::string> getPortList()
  {
    std::vector<std::string> ports;

#if defined(_WIN32)
    // --- Windows implementation using SetupAPI (preferred) ---
    HDEVINFO deviceInfoSet = SetupDiGetClassDevs(
        &GUID_DEVCLASS_PORTS, nullptr, nullptr, DIGCF_PRESENT);
    if (deviceInfoSet != INVALID_HANDLE_VALUE)
    {
      SP_DEVINFO_DATA devInfo;
      devInfo.cbSize = sizeof(devInfo);

      for (DWORD i = 0; SetupDiEnumDeviceInfo(deviceInfoSet, i, &devInfo); ++i)
      {
        char buffer[256];
        if (SetupDiGetDeviceRegistryPropertyA(
                deviceInfoSet, &devInfo, SPDRP_FRIENDLYNAME,
                nullptr, reinterpret_cast<PBYTE>(buffer), sizeof(buffer), nullptr))
        {
          std::string friendly(buffer);
          // Extract COM number, e.g. "USB-Serial Device (COM3)"
          std::smatch match;
          if (std::regex_search(friendly, match, std::regex("(COM\\d+)")))
            ports.push_back(match[1]);
        }
      }
      SetupDiDestroyDeviceInfoList(deviceInfoSet);
    }

    // Fallback scan if SetupAPI fails
    if (ports.empty())
    {
      for (int i = 1; i <= 32; ++i)
      {
        std::string port = "COM" + std::to_string(i);
        std::string path = "\\\\.\\" + port;
        HANDLE h = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE,
                               0, nullptr, OPEN_EXISTING, 0, nullptr);
        if (h != INVALID_HANDLE_VALUE)
        {
          ports.push_back(port);
          CloseHandle(h);
        }
      }
    }

#elif defined(__linux__) || defined(__APPLE__)
    // --- Linux, macOS, Raspberry Pi ---
    const std::vector<std::string> searchDirs = {"/dev/"};
    const std::regex pattern(
        "(ttyS\\d+|ttyUSB\\d+|ttyACM\\d+|ttyAMA\\d+|"
        "tty\\.usbserial.*|tty\\.usbmodem.*|cu\\..*)");

    for (const auto& dir : searchDirs)
    {
      for (const auto& entry : std::filesystem::directory_iterator(dir))
      {
        if (entry.is_character_file())
        {
          const std::string name = entry.path().filename().string();
          if (std::regex_match(name, pattern))
            ports.push_back(entry.path().string());
        }
      }
    }

#endif

    // --- Fallback defaults (for simulation or UI testing) ---
    if (ports.empty())
    {
#if defined(_WIN32)
      ports = {"COM1", "COM2"};
#else
      ports = {"/dev/ttyUSB0", "/dev/ttyACM0"};
#endif
    }

    return ports;
  }

  /**
   * \brief Check if a given port exists.
   */
  inline bool isValidPort(const std::string& port)
  {
    for (const auto& p : getPortList())
      if (p == port) return true;
    return false;
  }

  /**
   * \brief Try opening a port (optional lightweight check).
   */
  inline bool testOpen(const std::string& port)
  {
#if defined(_WIN32)
    std::string path = "\\\\.\\" + port;
    HANDLE h = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE,
                           0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;
    CloseHandle(h);
    return true;
#else
    FILE* f = fopen(port.c_str(), "r+");
    if (!f) return false;
    fclose(f);
    return true;
#endif
  }
}

#endif
