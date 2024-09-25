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
#include "../../utils/startswith.hpp"
#include "../../utils/setthreadname.hpp"

namespace MacOS {

static bool isSerialDevice(std::string_view devPath)
{
  return
    startsWith(devPath, "/dev/tty.") ||
    startsWith(devPath, "/dev/cu.");
}

SerialPortListImplIOKit::SerialPortListImplIOKit(SerialPortList& list)
  : SerialPortListImpl(list)
  , m_thread{
      [this]()
      {
        setThreadName("serialport-iokit");

        IONotificationPortRef notificationPort = IONotificationPortCreate(kIOMainPortDefault);
        CFRunLoopSourceRef runLoopSource = IONotificationPortGetRunLoopSource(notificationPort);
        CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopDefaultMode);

        io_iterator_t portIterator;
        kern_return_t result = findSerialPorts(&portIterator);
        if(result == KERN_SUCCESS)
        {
          handleDeviceIterator(portIterator, true); // add existing ports
        }

        // register for device add/remove notifications:
        io_iterator_t notificationIterator;
        IOServiceAddMatchingNotification(notificationPort, kIOFirstMatchNotification, getMatchingDictionary(), deviceAddedCallback, this, &notificationIterator);
        IOServiceAddMatchingNotification(notificationPort, kIOTerminatedNotification, getMatchingDictionary(), deviceRemovedCallback, this, &notificationIterator);

        CFRunLoopRun();

        // clean up:
        IOObjectRelease(notificationIterator);
        IONotificationPortDestroy(notificationPort);
      }}
{
}

SerialPortListImplIOKit::~SerialPortListImplIOKit()
{
  CFRunLoopStop(CFRunLoopGetCurrent());
  m_thread.join();
}

CFMutableDictionaryRef SerialPortListImplIOKit::getMatchingDictionary()
{
  CFMutableDictionaryRef matchingDict = IOServiceMatching(kIOSerialBSDServiceValue);
  if(matchingDict)
  {
    CFDictionarySetValue(matchingDict, CFSTR(kIOSerialBSDTypeKey), CFSTR(kIOSerialBSDAllTypes));
  }
  return matchingDict;
}

std::vector<std::string> SerialPortListImplIOKit::get() const
{
  std::vector<std::string> devices;
  io_iterator_t portIterator;
  kern_return_t result = findSerialPorts(&portIterator);

  if(result == KERN_SUCCESS)
  {
    handleDeviceIterator(portIterator, false, &devices);
  }

  return devices;
}

void SerialPortListImplIOKit::handleDeviceIterator(io_iterator_t iterator, bool notifyOnAdd, std::vector<std::string>* devices) const
{
  io_object_t device;
  while((device = IOIteratorNext(iterator)))
  {
    CFTypeRef devicePathAsCFString = IORegistryEntryCreateCFProperty(device, CFSTR(kIOCalloutDeviceKey), kCFAllocatorDefault, 0);
    if(devicePathAsCFString)
    {
      char devicePath[PATH_MAX];
      if(CFStringGetCString((CFStringRef)devicePathAsCFString, devicePath, PATH_MAX, kCFStringEncodingUTF8))
      {
        std::string devPath(devicePath);

        if(isSerialDevice(devPath))
        {
          if(notifyOnAdd)
          {
            EventLoop::call(
              [this, devPath]()
              {
                addToList(devPath);
              });
          }
          else if(devices)
          {
            devices->push_back(devPath);
          }
        }
      }
      CFRelease(devicePathAsCFString);
    }
    IOObjectRelease(device);
  }
}

void SerialPortListImplIOKit::deviceAddedCallback(void* refCon, io_iterator_t iterator)
{
  reinterpret_cast<SerialPortListImplIOKit*>(refCon)->handleDeviceIterator(iterator, true);
}

void SerialPortListImplIOKit::deviceRemovedCallback(void* refCon, io_iterator_t iterator)
{
  io_object_t device;
  while((device = IOIteratorNext(iterator)))
  {
    CFTypeRef devicePathAsCFString = IORegistryEntryCreateCFProperty(device, CFSTR(kIOCalloutDeviceKey), kCFAllocatorDefault, 0);
    if(devicePathAsCFString)
    {
      char devicePath[PATH_MAX];
      if(CFStringGetCString((CFStringRef)devicePathAsCFString, devicePath, PATH_MAX, kCFStringEncodingUTF8))
      {
        EventLoop::call(
          [refCon, devPath=std::string(devicePath)]()
          {
            reinterpret_cast<SerialPortListImplIOKit*>(refCon)->removeFromList(devPath);
          });
      }
      CFRelease(devicePathAsCFString);
    }
    IOObjectRelease(device);
  }
}

kern_return_t SerialPortListImplIOKit::findSerialPorts(io_iterator_t* portIterator)
{
  CFMutableDictionaryRef matchingDict = getMatchingDictionary();
  if(!matchingDict)
  {
    return KERN_FAILURE;
  }
  return IOServiceGetMatchingServices(kIOMainPortDefault, matchingDict, portIterator);
}

}
