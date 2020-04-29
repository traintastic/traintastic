/**
 * Traintastic
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

#include "usbxpressnetinterface.hpp"
#include "../../core/traintastic.hpp"
#include "../../core/world.hpp"

namespace Hardware::CommandStation {

USBXpressNetInterface::USBXpressNetInterface(const std::weak_ptr<World>& world, std::string_view _id) :
  CommandStation(world, _id),
  m_handle{nullptr},
  serial{this, "serial", "", PropertyFlags::ReadWrite},
  address{this, "address", 31, PropertyFlags::ReadWrite},
  xpressnet{this, "xpressnet", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject}
{
  name = "USB XpressNet interface";
  xpressnet.setValueInternal(std::make_shared<::Protocol::XpressNet>(*this, xpressnet.name(), std::bind(&USBXpressNetInterface::send, this, std::placeholders::_1)));

  m_interfaceItems.insertBefore(serial, notes);
  m_interfaceItems.insertBefore(address, notes);
  m_interfaceItems.insertBefore(xpressnet, notes);

  usbxpressnet_init();
}

USBXpressNetInterface::~USBXpressNetInterface()
{
  if(m_handle)
  {
    usbxpressnet_reset(m_handle);
    usbxpressnet_close(m_handle);
  }
  usbxpressnet_fini();
}

bool USBXpressNetInterface::setOnline(bool& value)
{
  if(!m_handle && value)
  {
    usbxpressnet_status status = usbxpressnet_open(!serial.value().empty() ? serial.value().c_str() : nullptr, &m_handle);
    if(status != USBXPRESSNET_STATUS_SUCCESS)
    {
      Traintastic::instance->console->error(id, std::string("usbxpressnet_open: ") + usbxpressnet_status_str(status));
      return false;
    }

    status = usbxpressnet_reset(m_handle);
    if(status != USBXPRESSNET_STATUS_SUCCESS)
    {
      Traintastic::instance->console->error(id, std::string("usbxpressnet_reset: ") + usbxpressnet_status_str(status));
      return false;
    }

    status = usbxpressnet_set_mode(m_handle, USBXPRESSNET_MODE_DEVICE, address);
    if(status != USBXPRESSNET_STATUS_SUCCESS)
    {
      Traintastic::instance->console->error(id, std::string("usbxpressnet_set_mode: ") + usbxpressnet_status_str(status));
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

void USBXpressNetInterface::emergencyStopChanged(bool value)
{

}

void USBXpressNetInterface::trackVoltageOffChanged(bool value)
{
}

void USBXpressNetInterface::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(online)
    xpressnet->decoderChanged(decoder, changes, functionNumber);
}

bool USBXpressNetInterface::send(const Protocol::XpressNet::Message& msg)
{
  assert(Protocol::XpressNet::isChecksumValid(msg));
  if(!m_handle)
    return false;
  usbxpressnet_status status;
  if((status = usbxpressnet_send_message(m_handle, &msg)) != USBXPRESSNET_STATUS_SUCCESS)
  {
    Traintastic::instance->console->critical(id, std::string("usbxpressnet_send_message: ") + usbxpressnet_status_str(status));
    return false;
  }
  return true;
}

}
