/**
 * server/src/utils/setthreadname.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_UTILS_SETTHREADNAME_HPP
#define TRAINTASTIC_SERVER_UTILS_SETTHREADNAME_HPP

#include <type_traits>
#include <thread>
#if __has_include(<pthread.h>)
  #include <pthread.h>
#endif

inline void setThreadName(const char* name)
{
#if __has_include(<pthread.h>)
  if constexpr(std::is_same_v<std::thread::native_handle_type, pthread_t>)
  #ifdef __APPLE__
    pthread_setname_np(name);
  #else
    pthread_setname_np(pthread_self(), name);
  #endif
#endif
#ifdef WIN32
  // TODO:
  //if constexpr(std::is_same_v<std::thread::native_handle_type, HANDLE>)
  //  SetThreadDescriptionA(GetCurrentThread(), name);
#endif
}

#endif
