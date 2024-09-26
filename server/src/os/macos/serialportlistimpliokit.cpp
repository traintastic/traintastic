/**
 * server/src/os/macos/serialportlistimpliokit.cpp
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

#include "serialportlistimpliokit.hpp"
#include <IOKit/serial/IOSerialKeys.h>
#include "../../core/eventloop.hpp"
#include "../../utils/setthreadname.hpp"

namespace MacOS {

static std::string getDevicePath(io_object_t device)
{
  std::string devicePath;
  CFTypeRef devicePathAsCFString = IORegistryEntryCreateCFProperty(device, CFSTR(kIOCalloutDeviceKey), kCFAllocatorDefault, 0);
  if(devicePathAsCFString)
  {
    devicePath.resize(PATH_MAX);
    if(CFStringGetCString((CFStringRef)devicePathAsCFString, devicePath.data(), devicePath.size(), kCFStringEncodingUTF8))
    {
      devicePath.resize(strnlen(devicePath.data(), PATH_MAX));
    }
    CFRelease(devicePathAsCFString);
  }
  return devicePath;
}

static CFMutableDictionaryRef getMatchingDictionary()
{
  CFMutableDictionaryRef matchingDict = IOServiceMatching(kIOSerialBSDServiceValue);
  if(matchingDict)
  {
    CFDictionarySetValue(matchingDict, CFSTR(kIOSerialBSDTypeKey), CFSTR(kIOSerialBSDAllTypes));
  }
  return matchingDict;
}

SerialPortListImplIOKit::SerialPortListImplIOKit(SerialPortList& list)
  : SerialPortListImpl(list)
  , m_thread{
      [this]()
      {
        setThreadName("serialport-iokit");

        m_runLoop = CFRunLoopGetCurrent();

        IONotificationPortRef notificationPort = IONotificationPortCreate(kIOMainPortDefault);
        CFRunLoopSourceRef runLoopSource = IONotificationPortGetRunLoopSource(notificationPort);
        CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopDefaultMode);

        // Register for device add/remove notifications:
        io_iterator_t notificationIterator;
        IOServiceAddMatchingNotification(notificationPort, kIOFirstMatchNotification, getMatchingDictionary(), deviceAddedCallback, this, &notificationIterator);
        IOServiceAddMatchingNotification(notificationPort, kIOTerminatedNotification, getMatchingDictionary(), deviceRemovedCallback, this, &notificationIterator);

        CFRunLoopRun();

        // Clean up:
        IOObjectRelease(notificationIterator);
        IONotificationPortDestroy(notificationPort);
      }}
{
}

SerialPortListImplIOKit::~SerialPortListImplIOKit()
{
  CFRunLoopStop(m_runLoop);
  m_thread.join();
}

std::vector<std::string> SerialPortListImplIOKit::get() const
{
  io_iterator_t iterator;
  kern_return_t result = IOServiceGetMatchingServices(kIOMainPortDefault, getMatchingDictionary(), &iterator);
  if(result != KERN_SUCCESS)
  {
    return {};
  }

  std::vector<std::string> devices;
  io_object_t device;
  while((device = IOIteratorNext(iterator)))
  {
    if(auto devPath = getDevicePath(device); !devPath.empty())
    {
      devices.push_back(devPath);
    }
    IOObjectRelease(device);
  }

  return devices;
}

void SerialPortListImplIOKit::deviceAddedCallback(void* refCon, io_iterator_t iterator)
{
  io_object_t device;
  while((device = IOIteratorNext(iterator)))
  {
    if(auto devPath = getDevicePath(device); !devPath.empty())
    {
      EventLoop::call(
        [refCon, devPath]()
        {
          reinterpret_cast<SerialPortListImplIOKit*>(refCon)->addToList(devPath);
        });
    }
    IOObjectRelease(device);
  }
}

void SerialPortListImplIOKit::deviceRemovedCallback(void* refCon, io_iterator_t iterator)
{
  io_object_t device;
  while((device = IOIteratorNext(iterator)))
  {
    if(auto devPath = getDevicePath(device); !devPath.empty())
    {
      EventLoop::call(
        [refCon, devPath]()
        {
          reinterpret_cast<SerialPortListImplIOKit*>(refCon)->removeFromList(devPath);
        });
    }
    IOObjectRelease(device);
  }
}

}
