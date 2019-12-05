/**
 * Traintastic
 *
 * Copyright (C) 2019 Reinder Feenstra <reinderfeenstra@gmail.com>
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
#include "protocol/xpressnet.hpp"
#include "../../core/traintastic.hpp"
#include "../../core/world.hpp"

namespace Hardware::CommandStation {

USBXpressNetInterface::USBXpressNetInterface(const std::weak_ptr<World>& world, const std::string& _id) :
  CommandStation(world, _id),
  m_handle{nullptr},
  serial{this, "serial", "", PropertyFlags::AccessWCC},
  address{this, "address", 31, PropertyFlags::TODO},
  xpressnet{this, "xpressnet", std::make_shared<Protocol::XpressNet>(world, world.lock()->getUniqueId("xpressnet"), std::bind(&USBXpressNetInterface::send, this, std::placeholders::_1)), PropertyFlags::AccessRRR}
{
  name = "USB XpressNet interface";

  m_interfaceItems.add(serial);
  m_interfaceItems.add(address);
  m_interfaceItems.add(xpressnet);

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

bool USBXpressNetInterface::isDecoderSupported(Decoder& decoder) const
{
  return xpressnet->isDecoderSupported(decoder);
}

void USBXpressNetInterface::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  xpressnet->decoderChanged(decoder, changes, functionNumber);
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

void USBXpressNetInterface::send(const void* msg)
{
  usbxpressnet_status status;
  if(m_handle && (status = usbxpressnet_send_message(m_handle, msg)) != USBXPRESSNET_STATUS_SUCCESS)
    Traintastic::instance->console->critical(id, std::string("usbxpressnet_send_message: ") + usbxpressnet_status_str(status));
}

}
