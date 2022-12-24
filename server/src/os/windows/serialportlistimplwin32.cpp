/**
 * server/src/os/linux/serialportlistimplwin32.cpp
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

#include "serialportlistimplwin32.hpp"
#include <array>
#include <boost/algorithm/string/predicate.hpp>
#include "../../core/eventloop.hpp" // include before windows.h, else we get winsock header issues
#include <windows.h>
#include <ntddser.h>
#include <setupapi.h>
#include <dbt.h>
#include "messagepump.hpp"
#include "registry.hpp"
#include "../../utils/startswith.hpp"
#include "../../utils/setthreadname.hpp"

namespace Windows {

SerialPortListImplWin32::SerialPortListImplWin32(SerialPortList& list)
  : SerialPortListImpl(list)
{
  MessagePump::start();
  MessagePump::setOnDeviceChangeComPort(
    [this](WPARAM event, std::string devicePath)
    {
      HKEY devices;
      if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\COM Name Arbiter\\Devices", 0, KEY_READ, &devices) != ERROR_SUCCESS)
        return;
      
      DWORD numberOfValues;
      DWORD maxValueNameLength;
      if(Registry::queryInfoKey(devices, numberOfValues, maxValueNameLength))
      {
        std::string port;
        std::string path;
        for(DWORD i = 0; i < numberOfValues; ++i)
        {
          port.resize(maxValueNameLength + 1); // add one for terminating zero
          if(Registry::enumValue(devices, i, port, path) && boost::iequals(devicePath, path))
          {
            switch(event)
            {
              case DBT_DEVICEARRIVAL:
                EventLoop::call(
                  [this, port]()
                  {
                    addToList(port);
                  });
                break;

              case DBT_DEVICEREMOVECOMPLETE:
                EventLoop::call(
                  [this, port]()
                  {
                    removeFromList(port);
                  });
                break;
            }
            break;
          }
        }
      }

      CloseHandle(devices);
    });
}

SerialPortListImplWin32::~SerialPortListImplWin32()
{
  MessagePump::setOnDeviceChangeComPort(nullptr);
  MessagePump::stop();
}

std::vector<std::string> SerialPortListImplWin32::get() const
{
  HDEVINFO devInfo = SetupDiGetClassDevsExA(&GUID_DEVINTERFACE_COMPORT, nullptr, nullptr, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT, nullptr, nullptr, nullptr);
  if(devInfo == INVALID_HANDLE_VALUE)
    return {};

  std::vector<std::string> ports;

  SP_DEVINFO_DATA devInfoData;
  for(DWORD index = 0; ; index++)
  {
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    if(!SetupDiEnumDeviceInfo(devInfo, index, &devInfoData))
      break;

    HKEY key = SetupDiOpenDevRegKey(devInfo, &devInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_QUERY_VALUE);
    if(key != INVALID_HANDLE_VALUE)
    {    
      std::string portName;
      if(Registry::queryValue(key, "PortName", portName) && startsWith(portName, "COM"))
      {
        ports.emplace_back(portName);
      }
      CloseHandle(key);
      
      //! \todo SetupDiGetDeviceRegistryPropertyA(devInfo, &devInfoData, SPDRP_FRIENDLYNAME, nullptr, &friendlyName, sizeof(friendlyName), nullptr) as alias?
    }
  }

  SetupDiDestroyDeviceInfoList(devInfo);

  return ports;
}

}
