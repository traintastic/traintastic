/**
 * server/src/os/windows/registry.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#include "registry.hpp"
#include <string>
#include <windows.h>
#include "../../utils/rtrim.hpp"

namespace Windows::Registry {

const char* runKey = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
//const char* runTraintasticServerKey = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\\TraintasticServer";
const char* startupApprovedKey = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run";
const char* traintasticServerKey = "TraintasticServer";
const WCHAR* traintasticServerKeyW = L"TraintasticServer";
const DWORD startupEnabled = 2;
const DWORD startupDisabled = 3;

bool addRun()
{
  bool success = false;
  HKEY key;
  LSTATUS r = RegOpenKeyExA(HKEY_CURRENT_USER, runKey, 0, KEY_WRITE, &key);
  if(r == ERROR_SUCCESS)
  {
    std::basic_string<WCHAR> path;
    path.resize(MAX_PATH);
    path[0] = '"';
    const DWORD l = GetModuleFileNameW(nullptr, path.data() + 1, path.size() - 1);
    if(l > 0)
    {
      path.resize(1 + l);
      path.append(L"\" --tray");

      r = RegSetValueExW(key, traintasticServerKeyW, 0, REG_SZ, reinterpret_cast<const BYTE*>(path.c_str()), (path.size() + 1) * sizeof(WCHAR));
      success = (r == ERROR_SUCCESS);
    }
    RegCloseKey(key);
  }
  return success;
}

bool getStartUpApproved(bool& enabled)
{
  bool success = false;
  HKEY key;
  LSTATUS r = RegOpenKeyExA(HKEY_CURRENT_USER, startupApprovedKey, 0, KEY_READ, &key);
  if(r == ERROR_SUCCESS)
  {
    DWORD data[3];
    DWORD size = sizeof(data);
    r = RegGetValueA(key, nullptr, traintasticServerKey, RRF_RT_REG_BINARY, nullptr, &data, &size);
    if(r == ERROR_SUCCESS)
    {
      enabled = (data[0] == startupEnabled);
      success = true;
    }
    RegCloseKey(key);
  }
  return success;
}

bool setStartUpApproved(bool enabled)
{
  bool success = false;
  HKEY key;
  LSTATUS r = RegOpenKeyExA(HKEY_CURRENT_USER, startupApprovedKey, 0, KEY_WRITE, &key);
  if(r == ERROR_SUCCESS)
  {
    DWORD data[3] = {enabled ? startupEnabled : startupDisabled, 0, 0};
    DWORD size = sizeof(data);
    r = RegSetValueExA(key, traintasticServerKey, 0, REG_BINARY, reinterpret_cast<const BYTE*>(&data), size);
    success = (r == ERROR_SUCCESS);
    RegCloseKey(key);
  }
  return success;
}

bool queryInfoKey(HKEY key, DWORD& numberOfValues, DWORD& maxValueNameLength)
{
  return RegQueryInfoKeyA(key, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &numberOfValues, &maxValueNameLength, nullptr, nullptr, nullptr) == ERROR_SUCCESS;
}

bool enumValue(HKEY key, DWORD index, std::string& name, std::string& value)
{
  DWORD type;
  DWORD nameSize = name.size();
  DWORD valueSize = value.size();

  if(RegEnumValueA(key, index, name.data(), &nameSize, nullptr, &type, reinterpret_cast<BYTE*>(value.data()), &valueSize) == ERROR_MORE_DATA && type == REG_SZ)
  {
    // add one for '\0'
    name.resize(++nameSize);
    value.resize(++valueSize);

    if(RegEnumValueA(key, index, name.data(), &nameSize, nullptr, nullptr, reinterpret_cast<BYTE*>(value.data()), &valueSize) != ERROR_SUCCESS)
      return false;

    name.resize(nameSize);
    value.resize(valueSize);
  }

  if(type != REG_SZ)
    return false;
  
  rtrim(value, '\0');
  return true;
}

bool queryValue(HKEY key, const char* name, std::string& value)
{
  DWORD type;
  DWORD size;

  if(RegQueryValueExA(key, name, nullptr, &type, nullptr, &size) == ERROR_SUCCESS && type == REG_SZ)
  {
    value.resize(size);
    if(RegQueryValueExA(key, name, nullptr, nullptr, reinterpret_cast<BYTE*>(value.data()), &size) == ERROR_SUCCESS)
    {
      rtrim(value, '\0');
      return true;
    }
  }

  return false;
}

}
