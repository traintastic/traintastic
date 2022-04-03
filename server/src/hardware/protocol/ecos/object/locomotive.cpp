/**
 * server/src/hardware/protocol/ecos/object/locomotive.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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

const std::initializer_list<std::string_view> Locomotive::options = {Option::addr, Option::protocol};

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
  send(get(m_id, {Option::dir, Option::speedStep}));
}

Locomotive::Locomotive(Kernel& kernel, const Line& data)
  : Locomotive(kernel, data.objectId)
{
  const auto values = data.values;
  if(auto addr = values.find(Option::addr); addr != values.end())
    fromChars(addr->second, m_address);
  if(auto protocol = values.find(Option::protocol); protocol != values.end())
    fromString(protocol->second, m_protocol);

  if(m_protocol != Protocol::Unknown)
    for(uint8_t i = 0; i < getFunctionCount(); i++)
      send(get(m_id, Option::func, i));
}

bool Locomotive::receiveReply(const Reply& reply)
{
  assert(reply.objectId == m_id);

  if(reply.command == Command::request && !reply.options.empty() && reply.options[0] == Option::control && reply.status == Status::Ok)
  {
    m_control = true;
  }

  return Object::receiveReply(reply);
}

bool Locomotive::receiveEvent(const Event& event)
{
  assert(event.objectId == m_id);

  return Object::receiveEvent(event);
}

void Locomotive::stop()
{
  send(set(m_id, {Option::stop}));
}

void Locomotive::setSpeedStep(uint8_t value)
{
  if(m_speedStep != value)
  {
    requestControl();
    send(set(m_id, Option::speedStep, value));
  }
}

void Locomotive::setDirection(Direction value)
{
  if(m_direction != value)
  {
    requestControl();
    send(set(m_id, Option::dir, (value == Direction::Reverse) ? 1 : 0));
  }
}

TriState Locomotive::getFunctionValue(uint8_t index) const
{
  if(auto it = m_functions.find(index); it != m_functions.end())
    return toTriState(it->second.value);
  return TriState::Undefined;
}

void Locomotive::setFunctionValue(uint8_t index, bool value)
{
  auto it = m_functions.find(index);
  if(it == m_functions.end() || it->second.value != value)
  {
    requestControl();
    send(set(m_id, Option::func, index, value ? 1 : 0));
  }
}

void Locomotive::update(std::string_view option, std::string_view value)
{
  if(option == Option::speedStep)
  {
    uint8_t v;
    fromChars(value, v);
    m_speedStep = v;
  }
  else if(option == Option::dir)
  {
    m_direction = (value == "1") ? Direction::Reverse : Direction::Forward;
  }
  else if(option == Option::func)
  {
    uint8_t fn;
    if(auto r = fromChars(value, fn); r.ec == std::errc() && r.ptr < value.data() + value.size() - 1 && *r.ptr == ',')
    {
      m_functions[fn].value = *(r.ptr + 1) == '1';
    }
  }
}

void Locomotive::requestControl()
{
  if(!m_control)
    send(request(m_id, {Option::control, Option::force}));
}

}
