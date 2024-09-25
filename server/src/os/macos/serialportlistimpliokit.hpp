/**
 * server/src/os/macos/serialportlistimpliokit.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_OS_MACOS_SERIALPORTLISTIMPLIOKIT_HPP
#define TRAINTASTIC_SERVER_OS_MACOS_SERIALPORTLISTIMPLIOKIT_HPP

#include "../serialportlistimpl.hpp"
#include <thread>
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>

namespace MacOS {

class SerialPortListImplIOKit : public SerialPortListImpl
{
private:
  static void deviceAddedCallback(void* refCon, io_iterator_t iterator);
  static void deviceRemovedCallback(void* refCon, io_iterator_t iterator);

  std::thread m_thread;

  void handleDeviceIterator(io_iterator_t iterator, bool notifyOnAdd, std::vector<std::string>* devices = nullptr) const;
  CFMutableDictionaryRef getMatchingDictionary();
  kern_return_t findSerialPorts(io_iterator_t* portIterator);

public:
  explicit SerialPortListImplIOKit(SerialPortList& list);
  ~SerialPortListImplIOKit();

  std::vector<std::string> get() const override;
};

}

#endif
