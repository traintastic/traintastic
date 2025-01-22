/**
 * server/src/hardware/protocol/ecos/object/ecos.cpp
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

#include "ecos.hpp"
#include <cassert>
#include "../kernel.hpp"
#include "../messages.hpp"
#include "../../../../utils/rtrim.hpp"
#include "../../../../utils/stripprefix.hpp"

namespace ECoS {

ECoS::ECoS(Kernel& kernel)
  : Object(kernel, ObjectId::ecos)
{
  requestView();
  send(get(m_id, {
    Option::commandstationtype,
    Option::protocolversion,
    Option::hardwareversion,
    Option::applicationversion,
    Option::applicationversionsuffix,
    Option::railcom,
    Option::railcomplus,
    }));
}

bool ECoS::receiveReply(const Reply& reply)
{
  assert(reply.objectId == m_id);

  if(reply.command == Command::set)
  {
    if(reply.options.size() == 1)
    {
      if(reply.options[0] == Option::stop)
      {
        if(m_go != TriState::False)
        {
          m_go = TriState::False;
          m_kernel.ecosGoChanged(m_go);
        }
      }
      else if(reply.options[0] == Option::go)
      {
        if(m_go != TriState::True)
        {
          m_go = TriState::True;
          m_kernel.ecosGoChanged(m_go);
        }
      }
    }
  }
  return Object::receiveReply(reply);
}

bool ECoS::receiveEvent(const Event& event)
{
  assert(event.objectId == m_id);

  return Object::receiveEvent(event);
}

void ECoS::go()
{
  if(m_go != TriState::True)
    send(set(m_id, {Option::go}));
}

void ECoS::stop()
{
  if(m_go != TriState::False)
    send(set(m_id, {Option::stop}));
}

void ECoS::update(std::string_view option, std::string_view value)
{
  if(option == Option::status)
  {
    if(value == "STOP")
    {
      if(m_go != TriState::False)
      {
        m_go = TriState::False;
        m_kernel.ecosGoChanged(m_go);
      }
    }
    else if(value == "GO")
    {
      if(m_go != TriState::True)
      {
        m_go = TriState::True;
        m_kernel.ecosGoChanged(m_go);
      }
    }
  }
  else if(option == Option::applicationversion)
  {
    Version::fromChars(value, m_applicationVersion);
  }
  else if(option == Option::applicationversionsuffix)
  {
    m_applicationVersionSuffix = std::string{value};
  }
  else if(option == Option::commandstationtype)
  {
    if(!fromString(value, m_model))
    {
      m_model = Model::Unknown;
    }
  }
  else if(option == Option::hardwareversion)
  {
    Version::fromChars(value, m_hardwareVersion);
  }
  else if(option == Option::protocolversion)
  {
    Version::fromChars(value, m_protocolVersion);
  }
  else if(option == Option::railcom)
  {
    m_railcom = (value == "1");
  }
  else if(option == Option::railcomplus)
  {
    m_railcomPlus = (value == "1");
  }
}

}
