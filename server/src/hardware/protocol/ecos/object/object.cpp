/**
 * server/src/hardware/protocol/ecos/object/object.cpp
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

#include "object.hpp"
#include <cassert>
#include "../messages.hpp"
#include "../kernel.hpp"

namespace ECoS {

Object::Object(Kernel& kernel, uint16_t id)
  : m_kernel{kernel}
  , m_id{id}
{
}

bool Object::receiveReply(const Reply& reply)
{
  assert(reply.objectId == m_id);

  if(reply.command == Command::get)
  {
    update(reply.lines);
  }
  else if(reply.command == Command::set && reply.status == Status::Ok)
  {
    std::string_view key;
    std::string_view value;
    for(auto option : reply.options)
      if(parseOptionValue(option, key, value))
        update(key, value);
  }

  return false;
}

bool Object::receiveEvent(const Event& event)
{
  assert(event.objectId == m_id);

  update(event.lines);

  return true;
}

void Object::requestView()
{
  if(!m_isViewActive)
    send(request(m_id, {Option::view}));
}

void Object::send(std::string_view message)
{
  m_kernel.send(message);
}

bool Object::objectExists(uint16_t objectId) const
{
  return m_kernel.objectExists(objectId);
}

void Object::addObject(std::unique_ptr<Object> object)
{
  m_kernel.addObject(std::move(object));
}

void Object::nameChanged()
{
  m_kernel.objectChanged(*this);
}

void Object::removeObject(uint16_t objectId)
{
  m_kernel.removeObject(objectId);
}

void Object::update(const std::vector<std::string_view>& lines)
{
  for(auto line : lines)
  {
    Line data;
    if(parseLine(line, data))
      for(auto it : data.values)
        update(it.first, it.second);
  }
}

}
