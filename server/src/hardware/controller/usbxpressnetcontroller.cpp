/**
 * server/src/hardware/controller/usbxpressnetcontroller.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra <reinderfeenstra@gmail.com>
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

#include "usbxpressnetcontroller.hpp"
#include "../../core/traintastic.hpp"
#include "../../world/world.hpp"
#include "../../core/attributes.hpp"

USBXpressNetController::USBXpressNetController(const std::weak_ptr<World>& world, std::string_view _id) :
  Controller(world, _id),
  m_handle{nullptr},
  serial{this, "serial", "", PropertyFlags::ReadWrite | PropertyFlags::Store},
  mode{this, "mode", USBXpressNetControllerMode::Direct, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  name = "USB XpressNet controller";

  m_interfaceItems.insertBefore(serial, notes);
  Attributes::addValues(mode, USBXpressNetControllerModeValues);
  m_interfaceItems.insertBefore(mode, notes);

  usbxpressnet_init();
}

USBXpressNetController::~USBXpressNetController()
{
  if(m_handle)
  {
    usbxpressnet_reset(m_handle);
    usbxpressnet_close(m_handle);
  }
  usbxpressnet_fini();
}

bool USBXpressNetController::setActive(bool& value)
{
  if(!m_handle && value)
  {
    usbxpressnet_status status = usbxpressnet_open(!serial.value().empty() ? serial.value().c_str() : nullptr, &m_handle);
    if(status != USBXPRESSNET_STATUS_SUCCESS)
    {
      // \todo Log::log(*this, LogMessage::E0001_X, std::string("usbxpressnet_open: ") + usbxpressnet_status_str(status));
      return false;
    }

    status = usbxpressnet_reset(m_handle);
    if(status != USBXPRESSNET_STATUS_SUCCESS)
    {
      // \todo Log::log(*this, LogMessage::E0001_X, std::string("usbxpressnet_reset: ") + usbxpressnet_status_str(status));
      return false;
    }

    status = usbxpressnet_set_mode(m_handle, USBXPRESSNET_MODE_STATION, 0);
    if(status != USBXPRESSNET_STATUS_SUCCESS)
    {
      // \todo Log::log(*this, LogMessage::E0001_X, std::string("usbxpressnet_set_mode: ") + usbxpressnet_status_str(status));
      return false;
    }
  }
  else if(m_handle && !value)
  {
    usbxpressnet_close(m_handle);
    m_handle = nullptr;
  }
  return true;
}

void USBXpressNetController::emergencyStopChanged(bool value)
{
}

void USBXpressNetController::trackPowerChanged(bool value)
{
}

void USBXpressNetController::decoderChanged(const Decoder& decoder, DecoderChangeFlags, uint32_t)
{
}
