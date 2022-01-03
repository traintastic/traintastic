/**
 * server/src/hardware/protocol/ecos/object/locomotive.cpp
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

#include "locomotive.hpp"
#include <cassert>
#include "../messages.hpp"
#include "../../../../utils/fromchars.hpp"

namespace ECoS {

const std::initializer_list<std::string_view> Locomotive::options = {Option::addr, Option::protocol, Option::dir, Option::speedStep};

static bool fromString(std::string_view text, Locomotive::Protocol& protocol)
{
  if(text == "MM14")
    protocol = Locomotive::Protocol::MM14;
  else if(text == "MM27")
    protocol = Locomotive::Protocol::MM27;
  else if(text == "MM28")
    protocol = Locomotive::Protocol::MM28;
  else if(text == "DCC14")
    protocol = Locomotive::Protocol::DCC14;
  else if(text == "DCC28")
    protocol = Locomotive::Protocol::DCC28;
  else if(text == "DCC128")
    protocol = Locomotive::Protocol::DCC128;
  else if(text == "SX32")
    protocol = Locomotive::Protocol::SX32;
  else if(text == "MMFKT")
    protocol = Locomotive::Protocol::MMFKT;
  else
    return false;
  return true;
}

Locomotive::Locomotive(Kernel& kernel, uint16_t id)
  : Object(kernel, id)
{
  requestView();
}

Locomotive::Locomotive(Kernel& kernel, const Line& data)
  : Locomotive(kernel, data.objectId)
{
  const auto values = data.values;
  if(auto addr = values.find(Option::addr); addr != values.end())
    fromChars(addr->second, m_address);
  if(auto protocol = values.find(Option::protocol); protocol != values.end())
    fromString(protocol->second, m_protocol);
  if(auto dir = values.find(Option::dir); dir != values.end())
  {}
  if(auto speedStep = values.find(Option::speedStep); speedStep != values.end())
  {}
}

bool Locomotive::receiveReply(const Reply& reply)
{
  assert(reply.objectId == m_id);

  return Object::receiveReply(reply);
}

bool Locomotive::receiveEvent(const Event& event)
{
  assert(event.objectId == m_id);

  return Object::receiveEvent(event);
}

}
