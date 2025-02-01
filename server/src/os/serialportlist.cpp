/**
 * server/src/os/serialportlist.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022,2024 Reinder Feenstra
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

#include "serialportlist.hpp"
#ifndef NDEBUG
  #include "../core/eventloop.hpp"
#endif
#ifdef HAS_LIBSYSTEMD
  #include "linux/serialportlistimplsystemd.hpp"
#elif defined(__linux__)
  #include "linux/serialportlistimplinotify.hpp"
#elif defined(WIN32)
  #include "windows/serialportlistimplwin32.hpp"
#else
  #include "serialportlistimpl.hpp"
#endif

SerialPortList& SerialPortList::instance()
{
  static std::unique_ptr<SerialPortList> instance = create();
  return *instance;
}

std::unique_ptr<SerialPortList> SerialPortList::create()
{
  std::unique_ptr<SerialPortList> list{new SerialPortList()};
  list->m_impl = std::make_unique<Impl>(*list);
  list->m_list = list->m_impl->get();
  return list;
}

SerialPortList::~SerialPortList() = default;

void SerialPortList::add(std::string value)
{
  assert(isEventLoopThread());
  m_list.emplace_back(std::move(value));
  changed();
}

void SerialPortList::remove(std::string_view value)
{
  assert(isEventLoopThread());
  bool removed = false;
  for(auto it = m_list.begin(); it != m_list.end();)
  {
    if(*it == value)
    {
      it = m_list.erase(it);
      removed = true;
    }
    else
      it++;
  }

  if(removed)
    changed();
}
