/**
 * shared/src/traintastic/utils/standardpaths.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2023 Reinder Feenstra
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
#ifdef WIN32
  #include <windows.h>
  #include <shlobj.h>
#endif

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
#if defined(WIN32) && !(defined(__MINGW32__) || defined(__MINGW64__))
  wchar_t* path = nullptr;
  size_t pathLength = 0;
  if(_wdupenv_s(&path, &pathLength, L"TRAINTASTIC_LOCALE_PATH") == 0 && path && pathLength != 0)
  {
    std::filesystem::path p(path);
    free(path);
    return p;
  }
#else
  if(const char* path = getenv("TRAINTASTIC_LOCALE_PATH"))
    return std::filesystem::path(path);
#endif

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
#ifdef WIN32
  return getProgramDataPath() / "traintastic" / "manual";
#else
  return {};
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
