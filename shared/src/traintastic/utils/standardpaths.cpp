/**
 * shared/src/traintastic/utils/standardpaths.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2024 Reinder Feenstra
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

#include "standardpaths.hpp"
#include <optional>
#ifdef WIN32
  #include <windows.h>
  #include <shlobj.h>
#endif

namespace {

#if defined(WIN32) && !(defined(__MINGW32__) || defined(__MINGW64__))

#define getEnvironmentVariableAsPath(name) getEnvironmentVariableAsPathImpl(L ## name)

std::optional<std::filesystem::path> getEnvironmentVariableAsPathImpl(const wchar_t* name)
{
  wchar_t* value = nullptr;
  size_t valueLength = 0;
  if(_wdupenv_s(&value, &valueLength, name) == 0 && value && valueLength != 0)
  {
    std::filesystem::path path(value);
    free(value);
    return path;
  }
  return std::nullopt;
}

#else

#define getEnvironmentVariableAsPath(name) getEnvironmentVariableAsPathImpl(name)

std::optional<std::filesystem::path> getEnvironmentVariableAsPathImpl(const char* name)
{
  if(const char* value = getenv(name))
  {
    return std::filesystem::path(value);
  }
  return std::nullopt;
}

#endif

}

#ifdef WIN32
static std::filesystem::path getKnownFolderPath(REFKNOWNFOLDERID rfid)
{
  std::filesystem::path path;
  PWSTR localAppDataPath = nullptr;
  HRESULT r = SHGetKnownFolderPath(rfid, 0, nullptr, &localAppDataPath);
  if(r == S_OK)
    path = std::filesystem::path(localAppDataPath);
  if(localAppDataPath)
    CoTaskMemFree(localAppDataPath);
  return path;
}

std::filesystem::path getProgramDataPath()
{
  return getKnownFolderPath(FOLDERID_ProgramData);
}

std::filesystem::path getLocalAppDataPath()
{
  return getKnownFolderPath(FOLDERID_LocalAppData);
}
#endif

std::filesystem::path getLocalePath()
{
  if(auto path = getEnvironmentVariableAsPath("TRAINTASTIC_LOCALE_PATH"))
  {
    return *path;
  }
#ifdef WIN32
  return getProgramDataPath() / "traintastic" / "translations";
#elif defined(__linux__)
  return "/opt/traintastic/translations";
#else
  return std::filesystem::current_path() / "translations";
#endif
}

std::filesystem::path getManualPath()
{
  if(auto path = getEnvironmentVariableAsPath("TRAINTASTIC_MANUAL_PATH"))
  {
    return *path;
  }
#ifdef WIN32
  return getProgramDataPath() / "traintastic" / "manual";
#elif defined(__linux__)
  return "/opt/traintastic/manual";
#else
  return std::filesystem::current_path() / "manual";
#endif
}

std::filesystem::path getLNCVXMLPath()
{
#ifdef WIN32
  return getProgramDataPath() / "traintastic" / "lncv";
#elif defined(__linux__)
  return "/opt/traintastic/lncv";
#else
  return std::filesystem::current_path() / "lncv";
#endif
}
