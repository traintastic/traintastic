/**
 * server/src/hardware/protocol/ecos/object/switch.cpp
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

#include "switch.hpp"
#include <cassert>
#include "../messages.hpp"
#include "../kernel.hpp"
#include "../../../../utils/fromchars.hpp"
#include "../../../../utils/inrange.hpp"

namespace ECoS {

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

  send(get(m_id, {
    Option::name1,
    Option::name2,
    Option::name3,
    Option::type,
    Option::symbol,
    Option::addr,
    Option::protocol,
    Option::state,
    Option::mode,
    Option::duration,
    Option::variant,
    }));
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

void Switch::setState(uint8_t value)
{
  send(set(m_id, Option::state, value));
}

void Switch::update(std::string_view option, std::string_view value)
{
  if(option == Option::name1)
  {
    m_name1 = value;
  }
  else if(option == Option::name2)
  {
    m_name2 = value;
  }
  else if(option == Option::name3)
  {
    m_name3 = value;
  }
  else if(option == Option::addr)
  {
    fromChars(value, m_address);
  }
  else if(option == Option::symbol)
  {
    std::underlying_type_t<Symbol> n = 0;
    fromChars(value, n);
    m_symbol = static_cast<Symbol>(n);
  }
  else if(option == Option::type)
  {
    fromString(value, m_type);
  }
  else if(option == Option::protocol)
  {
    fromString(value, m_protocol);
  }
  else if(option == Option::state)
  {
    fromChars(value, m_state);

    // calulate switched accessory (needs verification):
    m_kernel.switchManagerSwitched(m_protocol, m_address + (m_state / 2), (m_state & 0x01) ? OutputPairValue::First : OutputPairValue::Second);
  }
  else if(option == Option::mode)
  {
    fromString(value, m_mode);
  }
  else if(option == Option::duration)
  {
    fromChars(value, m_duration);
  }
  else if(option == Option::variant)
  {
    fromChars(value, m_variant);
  }
}

}
