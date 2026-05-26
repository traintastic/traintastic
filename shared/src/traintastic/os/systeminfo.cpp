/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "systeminfo.hpp"
#include <cstdlib>
#include <thread>

#ifdef _WIN32
  #include <windows.h>
  #include <psapi.h>
  #include <array>
#endif

#if defined(__unix__) || defined(__APPLE__)
  #include <sys/utsname.h>
#endif

#if defined(__linux__)
  #include <fstream>
  #include <unistd.h>
#endif

namespace {

#ifdef _WIN32
void appendSystemInfoWindows(std::string& info)
{
  info.append("OS: Windows\n");

  // Windows version
  if (HMODULE ntdll = GetModuleHandleW(L"ntdll.dll"))
  {
    using RtlGetVersionPtr = LONG (WINAPI* )(PRTL_OSVERSIONINFOW);
    if(auto RtlGetVersion = reinterpret_cast<RtlGetVersionPtr>(reinterpret_cast<void*>(GetProcAddress(ntdll, "RtlGetVersion"))))
    {
      RTL_OSVERSIONINFOW version;
      ZeroMemory(&version, sizeof(version));
      version.dwOSVersionInfoSize = sizeof(version);
      if(RtlGetVersion(&version) == 0)
      {
        info.append("Version: ")
          .append(std::to_string(version.dwMajorVersion))
          .append(".")
          .append(std::to_string(version.dwMinorVersion))
          .append(" (build: ")
          .append(std::to_string(version.dwBuildNumber))
          .append(")\n");
      }
    }
  }

  // Windows friendly name
  {
    HKEY localMachine;
    if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &localMachine) == ERROR_SUCCESS)
    {
      std::array<char, 128> value;
      DWORD length = value.size();

      if(RegGetValueA(localMachine, nullptr, "DisplayVersion", RRF_RT_REG_SZ, nullptr, value.data(), &length) == ERROR_SUCCESS)
      {
        info.append("DisplayVersion: ").append(value.data(), length).append("\n");
      }
      else // fallback (older Windows 10)
      {
        length = value.size();
        if(RegGetValueA(localMachine, nullptr, "ReleaseId", RRF_RT_REG_SZ, nullptr, value.data(), &length) == ERROR_SUCCESS)
        {
          info.append("ReleaseId: ").append(value.data(), length).append("\n");
        }
      }
      RegCloseKey(localMachine);
    }
  }

  // System memory
  {
    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    if(GlobalMemoryStatusEx(&mem))
    {
      info.append("Memory total: ").append(std::to_string(mem.ullTotalPhys / (1024 * 1024))).append(" MB\n");
      info.append("Memory available: ").append(std::to_string(mem.ullAvailPhys / (1024 * 1024))).append(" MB\n");
    }
  }
  
  // Process memory
  {
    PROCESS_MEMORY_COUNTERS pmc;
    if(GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
    {
      info.append("Process memory: ").append(std::to_string(pmc.WorkingSetSize / (1024 * 1024))).append(" MB\n");
    }
  }
}
#endif

#if defined(__unix__) || defined(__APPLE__)
void appendSystemInfoUname(std::string& info)
{
  struct utsname buf;
  if(uname(&buf) == 0)
  {
    info.append("OS: ").append(buf.sysname).append("\n");
    info.append("Release: ").append(buf.release).append("\n");
    info.append("Version: ").append(buf.version).append("\n");
    info.append("Machine: ").append(buf.machine).append("\n");
  }
}
#endif

#ifdef __linux__
void appendSystemInfoLinux(std::string& info)
{
  appendSystemInfoUname(info);

  // Distro:
  {
    std::ifstream file("/etc/os-release");
    std::string line;

    while(std::getline(file, line))
    {
      if(line.starts_with("PRETTY_NAME="))
      {
        info.append("Distro: ").append(line.substr(13, line.size() - 14)).append("\n");
        break;
      }
    }
  }

  // System memory:
  {
    std::ifstream file("/proc/meminfo");
    std::string key;
    long value;
    std::string unit;

    long total = 0, available = 0;

    while(file >> key >> value >> unit)
    {
      if (key == "MemTotal:")
      {
        total = value;
      }
      if (key == "MemAvailable:")
      {
        available = value;
      }
    }

    if(total > 0)
    {
      info.append("Memory total: ").append(std::to_string(total / 1024)).append(" MB\n");
    }
    if(available > 0)
    {
      info.append("Memory available: ").append(std::to_string(available / 1024)).append(" MB\n");
    }
  }

  // Process memory
  {
    std::ifstream statm("/proc/self/statm");
    long size, resident;
    statm >> size >> resident;

    long page_size = sysconf(_SC_PAGESIZE);
    long rss_mb = (resident * page_size) / (1024 * 1024);

    info.append("Process memory: ").append(std::to_string(rss_mb)).append(" MB\n");
  }
}
#endif

#ifdef __APPLE__
void appendSystemInfoMacOS(std::string& info)
{
  appendSystemInfoUname(info);

  // FIXME: get system info
}
#endif

void appendCompilerInfo(std::string& info)
{
  info.append("\n### Compiler ###\n");

#if defined(__clang__) // Clang
  #if defined(__apple_build_version__)
  info.append("Compiler: Clang (Apple)\n");
  #else
  info.append("Compiler: Clang\n");
  #endif
  info.append("Version: ").append(__clang_version__).append("\n");
#elif defined(__GNUC__) // GCC (note: also defines __GNUC__)
  #if defined(__MINGW32__) || defined(__MINGW64__)
  info.append("Compiler: GCC (MinGW)\n");
  #else
  info.append("Compiler: GCC\n");
  #endif
  info.append("Version: ")
    .append(std::to_string(__GNUC__))
    .append(".")
    .append(std::to_string(__GNUC_MINOR__))
    .append(".")
    .append(std::to_string(__GNUC_PATCHLEVEL__))
    .append("\n");
#elif defined(_MSC_VER) // MSVC
  info.append("Compiler: MSVC\n");
  info.append("Version: ")
    .append(std::to_string(_MSC_VER / 100))
    .append(".")
    .append(std::to_string(_MSC_VER % 100))
    .append("\n");
  #if defined(_MSC_FULL_VER)
  info.append("Full Version: ").append(std::to_string(_MSC_FULL_VER)).append("\n");
  #endif
#else
  info.append("Compiler: Unknown\n");
#endif

#if defined(__x86_64__) || defined(_M_X64)
  info.append("Architecture: x86_64\n");
#elif defined(__i386__) || defined(_M_IX86)
  info.append("Architecture: x86\n");
#elif defined(__aarch64__) || defined(_M_ARM64)
  info.append("Architecture: ARM64\n");
#elif defined(__arm__) || defined(_M_ARM)
  info.append("Architecture: ARM\n");
#else
  info.append("Architecture: Unknown\n");
#endif
}

}

#if defined(_WIN32)
  extern char** _environ;
  #define environ _environ
#endif

#ifdef __APPLE__
  extern char** environ;
#endif

std::string getSystemInfo()
{
  std::string info("### System info ###\n");

#if defined(_WIN32)
  appendSystemInfoWindows(info);
#elif defined(__linux__)
  appendSystemInfoLinux(info);
#elif defined(__APPLE__)
  appendSystemInfoMacOS(info);
#else
    out).append("OS: Unknown\n";
#endif
  info.append("CPU cores: ").append(std::to_string(std::thread::hardware_concurrency())).append("\n");

  info.append("\n### Environment (TRAINTASTIC_*) ###\n");
  for(char** env = environ; *env != nullptr; ++env)
  {
    std::string_view var(*env);
    if(var.starts_with("TRAINTASTIC_"))
    {
      info.append(var).append("\n");
    }
  }

  appendCompilerInfo(info);

  return info;
}
