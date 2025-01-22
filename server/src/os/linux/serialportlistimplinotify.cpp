/**
 * server/src/os/linux/serialportlistimplinotify.cpp
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

#include "serialportlistimplinotify.hpp"
#include "isserialdevice.hpp"
#include <sys/inotify.h>
#include <poll.h>
#include <filesystem>
#include "../../core/eventloop.hpp"
#include "../../utils/startswith.hpp"
#include "../../utils/setthreadname.hpp"

namespace Linux {

SerialPortListImplInotify::SerialPortListImplInotify(SerialPortList& list)
  : SerialPortListImpl(list)
  , m_stopEvent{eventfd(0, O_NONBLOCK)}
  , m_thread{
      [this]()
      {
        setThreadName("serialport-inotify");

        int inotifyFd = inotify_init1(IN_NONBLOCK);
        if(inotifyFd == -1)
        {
          return;
        }

        int watchFd = inotify_add_watch(inotifyFd, "/dev", IN_CREATE | IN_DELETE);
        if(watchFd == -1)
        {
          close(inotifyFd);
          return;
        }

        // Polling for events:
        pollfd fds[2];
        fds[0].fd = inotifyFd;
        fds[0].events = POLLIN;
        fds[1].fd = m_stopEvent;
        fds[1].events = POLLIN;

        for(;;)
        {
          int r = poll(fds, 2, -1); // wait for events
          if(r == -1)
          {
            if(errno == EINTR)
            {
              continue; // interrupted by signal
            }
            break; // poll failed
          }

          if(fds[1].revents & POLLIN) // stop event
          {
            break;
          }
          else if(fds[0].revents & POLLIN) // inotify event
          {
            handleInotifyEvents(inotifyFd);
          }
        }

        // clean up:
        inotify_rm_watch(inotifyFd, watchFd);
        close(inotifyFd);
      }}
{
}

SerialPortListImplInotify::~SerialPortListImplInotify()
{
  eventfd_write(m_stopEvent, 1);
  m_thread.join();
  close(m_stopEvent);
}

void SerialPortListImplInotify::handleInotifyEvents(int inotifyFd)
{
  char buffer[1024];
  ssize_t length = read(inotifyFd, buffer, sizeof(buffer));
  if(length < 0)
  {
    return;
  }

  for(ssize_t i = 0; i < length;)
  {
    const auto* event = reinterpret_cast<const inotify_event*>(&buffer[i]);
    const auto devPath = std::string("/dev/") + event->name;

    if(event->mask & IN_CREATE)
    {
      if(isSerialDevice(devPath))
      {
        EventLoop::call(
          [this, devPath]()
          {
            addToList(devPath);
          });
      }
    }
    else if(event->mask & IN_DELETE)
    {
      if(isSerialDevice(devPath))
      {
        EventLoop::call(
          [this, devPath]()
          {
            removeFromList(devPath);
          });
      }
    }

    i += sizeof(inotify_event) + event->len;
  }
}

std::vector<std::string> SerialPortListImplInotify::get() const
{
  std::vector<std::string> devices;
  const std::filesystem::path devDir("/dev");

  if(std::filesystem::exists(devDir) && std::filesystem::is_directory(devDir))
  {
    for(const auto& entry : std::filesystem::directory_iterator(devDir))
    {
      const auto& devPath = entry.path().string();
      if(isSerialDevice(devPath))
      {
        devices.push_back(devPath);
      }
    }
  }

  return devices;
}

}
