/**
 * server/src/utils/setthreadname.cpp
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

#include "setthreadname.hpp"
#include <type_traits>
#include <thread>
#include <string>
#if __has_include(<pthread.h>)
  #include <pthread.h>
#endif
#ifdef WIN32
  #include <windows.h>
  #include <processthreadsapi.h>
#endif

void setThreadName(const char* name)
{
#if __has_include(<pthread.h>)
  if constexpr(std::is_same_v<std::thread::native_handle_type, pthread_t>)
  #ifdef __APPLE__
    pthread_setname_np(name);
  #else
    pthread_setname_np(pthread_self(), name);
  #endif
#endif
#if defined(WIN32) && !defined(__MINGW32__) && !defined(__MINGW64__)
  if constexpr(std::is_same_v<std::thread::native_handle_type, HANDLE>)
  {
    const size_t nameSize = strlen(name);
    std::basic_string<WCHAR> wideName;
    wideName.resize(nameSize);
    if(MultiByteToWideChar(CP_UTF8, 0, name, static_cast<int>(nameSize), wideName.data(), static_cast<int>(wideName.size())) > 0)
      SetThreadDescription(GetCurrentThread(), wideName.c_str());
  }
#endif
}
