/**
 * server/src/os/linux/serialportlistimplsystemd.cpp
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

#include "serialportlistimplsystemd.hpp"
#include "../../core/eventloop.hpp"
#include "../../utils/startswith.hpp"
#include "../../utils/setthreadname.hpp"

namespace Linux {

static std::string_view getPropertyValue(sd_device* device, const char* key)
{
  const char* value;
  if(sd_device_get_property_value(device, key, &value) == 0)
    return value;
  return {};
}

static std::string_view getDevPath(sd_device* device)
{
  return getPropertyValue(device, "DEVNAME");
}

static bool isSerialDevice(sd_device* device)
{
  auto devPath = getDevPath(device);
  return
    startsWith(devPath, "/dev/ttyS") ||
    startsWith(devPath, "/dev/ttyUSB") ||
    startsWith(devPath, "/dev/ttyACM");
}


SerialPortListImplSystemD::SerialPortListImplSystemD(SerialPortList& list)
  : SerialPortListImpl(list)
  , m_stopEvent{eventfd(0, O_NONBLOCK)}
  , m_thread{
      [this]()
      {
        setThreadName("serialport-sysd");

        sd_device_monitor* monitor = nullptr;

        if(sd_device_monitor_new(&monitor) == 0)
        {
          sd_device_monitor_filter_add_match_subsystem_devtype(monitor, "tty", nullptr);

          if(sd_device_monitor_start(monitor, monitorHandlerHelper, this) == 0)
          {
            sd_event* eventLoop = sd_device_monitor_get_event(monitor);
            sd_event_source* src;
            sd_event_add_io(eventLoop, &src, m_stopEvent, EPOLLIN, stopEventHandler, this);

            sd_event_loop(eventLoop);

            sd_event_source_unref(src);
            sd_device_monitor_stop(monitor);
          }
          sd_device_monitor_unref(monitor);
        }
      }}
{
}

SerialPortListImplSystemD::~SerialPortListImplSystemD()
{
  eventfd_write(m_stopEvent, 1);
  m_thread.join();
  close(m_stopEvent);
}

std::vector<std::string> SerialPortListImplSystemD::get() const
{
  std::vector<std::string> devices;

  sd_device_enumerator* enumerator = nullptr;
  if(sd_device_enumerator_new(&enumerator) == 0)
  {
    sd_device_enumerator_add_match_subsystem(enumerator, "tty", 1);

    sd_device* device = sd_device_enumerator_get_device_first(enumerator);
    while(device)
    {
      if(isSerialDevice(device))
      {
        devices.emplace_back(std::string{getDevPath(device)});
      }

      device = sd_device_enumerator_get_device_next(enumerator);
    }

    enumerator = sd_device_enumerator_unref(enumerator);
  }

  return devices;
}

int SerialPortListImplSystemD::monitorHandler(sd_device* device)
{
  if(isSerialDevice(device))
  {
    auto action = getPropertyValue(device, "ACTION");
    if(action == "add")
    {
      EventLoop::call(
        [this, devPath=std::string{getDevPath(device)}]()
        {
          addToList(devPath);
        });
    }
    else if(action == "remove")
    {
      EventLoop::call(
        [this, devPath=std::string{getDevPath(device)}]()
        {
          removeFromList(devPath);
        });
    }
  }

  return 0;
}

}
