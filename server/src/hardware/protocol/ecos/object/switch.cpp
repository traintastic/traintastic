/**
 * server/src/hardware/protocol/ecos/object/switch.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#include "switch.hpp"
#include <cassert>
#include "../messages.hpp"
#include "../../../../utils/fromchars.hpp"

namespace ECoS {

const std::initializer_list<std::string_view> Switch::options = {Option::addr, Option::protocol, Option::state, Option::mode, Option::duration};

static bool fromString(std::string_view text, Switch::Protocol& protocol)
{
  if(text == "MM")
    protocol = Switch::Protocol::MM;
  else if(text == "DCC")
    protocol = Switch::Protocol::DCC;
  else
    return false;
  return true;
}

static bool fromString(std::string_view text, Switch::Mode& mode)
{
  if(text == "SWITCH")
    mode = Switch::Mode::Switch;
  else if(text == "PULSE")
    mode = Switch::Mode::Pulse;
  else
    return false;
  return true;
}

Switch::Switch(Kernel& kernel, uint16_t id)
  : Object(kernel, id)
{
  requestView();
}

Switch::Switch(Kernel& kernel, const Line& data)
  : Switch(kernel, data.objectId)
{
  const auto values = data.values;
  if(auto addr = values.find(Option::addr); addr != values.end())
    fromChars(addr->second, m_address);
  if(auto protocol = values.find(Option::protocol); protocol != values.end())
    fromString(protocol->second, m_protocol);
  if(auto state = values.find(Option::state); state != values.end())
  {}
  if(auto mode = values.find(Option::mode); mode != values.end())
    fromString(mode->second, m_mode);
  if(auto duration = values.find(Option::duration); duration != values.end())
    fromChars(duration->second, m_duration);
}

bool Switch::receiveReply(const Reply& reply)
{
  assert(reply.objectId == m_id);

  (void)(reply);

  return Object::receiveReply(reply);
}

bool Switch::receiveEvent(const Event& event)
{
  assert(event.objectId == m_id);

  (void)(event);

  return Object::receiveEvent(event);
}

}
