/**
 * server/src/os/linux/serialportlistimplwin32.hpp
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

#ifndef TRAINTASTIC_SERVER_OS_LINUX_SERIALPORTLISTIMPLWIN32_HPP
#define TRAINTASTIC_SERVER_OS_LINUX_SERIALPORTLISTIMPLWIN32_HPP

#include "../serialportlistimpl.hpp"
#include <thread>

namespace Windows {

class SerialPortListImplWin32 final : public SerialPortListImpl
{
  private:
    std::thread m_thread;

  public:
    SerialPortListImplWin32(SerialPortList& list);
    ~SerialPortListImplWin32() final;

    std::vector<std::string> get() const final;
};

}

#endif
