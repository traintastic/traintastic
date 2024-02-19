/**
 * server/src/hardware/protocol/ecos/object/switchmanager.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022,2024 Reinder Feenstra
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

#include "switchmanager.hpp"
#include <cassert>
#include "switch.hpp"
#include "../kernel.hpp"
#include "../messages.hpp"
#include "../../../../utils/fromchars.hpp"
#include "../../../../utils/startswith.hpp"

namespace ECoS {

SwitchManager::SwitchManager(Kernel& kernel)
  : Object(kernel, ObjectId::switchManager)
{
  requestView();
  send(queryObjects(m_id, Switch::options));
}

void SwitchManager::setSwitch(SwitchProtocol protocol, uint16_t address, bool port)
{
  if(protocol == SwitchProtocol::DCC || protocol == SwitchProtocol::Motorola) /*[[likely]]*/
    send(set(m_id, Option::switch_, std::string((protocol == SwitchProtocol::Motorola) ? "MOT" : "DCC").append(std::to_string(address)).append(port ? "g" : "r")));
}

bool SwitchManager::receiveReply(const Reply& reply)
{
  assert(reply.objectId == m_id);

  if(reply.command == Command::queryObjects)
  {
    for(std::string_view line : reply.lines)
    {
      Line data;
      if(parseLine(line, data) && !objectExists(data.objectId))
        addObject(std::make_unique<Switch>(m_kernel, data));
    }
    return true;
  }

  return Object::receiveReply(reply);
}

bool SwitchManager::receiveEvent(const Event& event)
{
  assert(event.objectId == m_id);


  return Object::receiveEvent(event);
}

void SwitchManager::update(std::string_view option, std::string_view value)
{
  if(option == Option::switch_)
  {
    auto protocol = SwitchProtocol::Unknown;
    for(auto p : {SwitchProtocol::DCC, SwitchProtocol::Motorola})
      if(startsWith(value, toString(p)))
      {
        protocol = p;
        value = value.substr(toString(p).size());
        break;
      }

    if(protocol != SwitchProtocol::Unknown)
    {
      uint16_t address;
      if(auto r = fromChars(value, address); r.ec == std::errc() && r.ptr < value.data() + value.size())
      {
        m_kernel.switchManagerSwitched(protocol, address, (*r.ptr == 'r') ? OutputPairValue::First : OutputPairValue::Second);
      }
    }
  }
}

}
