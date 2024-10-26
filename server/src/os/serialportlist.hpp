/**
 * server/src/os/serialportlist.hpp
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

#ifndef TRAINTASTIC_SERVER_OS_SERIALPORTLIST_HPP
#define TRAINTASTIC_SERVER_OS_SERIALPORTLIST_HPP

#include <memory>
#include <vector>
#include <string>
#include <boost/signals2/signal.hpp>

class SerialPortListImpl;
#ifdef HAS_LIBSYSTEMD
namespace Linux {
  class SerialPortListImplSystemD;
}
#elif defined(__linux__)
namespace Linux {
  class SerialPortListImplInotify;
}
#elif defined(WIN32)
namespace Windows {
  class SerialPortListImplWin32;
}
#endif

class SerialPortList
{
  friend class SerialPortListImpl;

  private:
#ifdef HAS_LIBSYSTEMD
    using Impl = Linux::SerialPortListImplSystemD;
#elif defined(__linux__)
    using Impl = Linux::SerialPortListImplInotify;
#elif defined(WIN32)
    using Impl = Windows::SerialPortListImplWin32;
#else
    using Impl = SerialPortListImpl;
#endif

    static std::unique_ptr<SerialPortList> create();

    std::unique_ptr<Impl> m_impl;
    std::vector<std::string> m_list;

    SerialPortList() = default;

    void add(std::string value);
    void remove(std::string_view value);

  public:
    static SerialPortList& instance();

    boost::signals2::signal<void()> changed;

    ~SerialPortList();

    inline const std::vector<std::string>& get() const
    {
      return m_list;
    }
};

#endif
