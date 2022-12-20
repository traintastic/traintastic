/**
 * server/src/os/linux/serialportlistimplsystemd.hpp
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

#ifndef TRAINTASTIC_SERVER_OS_LINUX_SERIALPORTLISTIMPLSYSTEMD_HPP
#define TRAINTASTIC_SERVER_OS_LINUX_SERIALPORTLISTIMPLSYSTEMD_HPP

#include "../serialportlistimpl.hpp"
#include <thread>
#include <systemd/sd-device.h>

namespace Linux {

class SerialPortListImplSystemD final : public SerialPortListImpl
{
  private:
    static int monitorHandlerHelper(sd_device_monitor* /*m*/, sd_device* device, void* userdata)
    {
      return reinterpret_cast<SerialPortListImplSystemD*>(userdata)->monitorHandler(device);
    }

    static int stopEventHandler(sd_event_source* s, int /*fd*/, uint32_t /*revents*/, void* /*userdata*/)
    {
      sd_event_exit(sd_event_source_get_event(s), 0);
      return 0;
    }

    int m_stopEvent;
    std::thread m_thread;

    int monitorHandler(sd_device* device);

  public:
    SerialPortListImplSystemD(SerialPortList& list);
    ~SerialPortListImplSystemD() final;

    std::vector<std::string> get() const final;
};

}

#endif
