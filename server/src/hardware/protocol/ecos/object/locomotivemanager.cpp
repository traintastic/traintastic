/**
 * server/src/hardware/protocol/ecos/object/locomotivemanager.cpp
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

#include "locomotivemanager.hpp"
#include <cassert>
#include "locomotive.hpp"
#include "../messages.hpp"

namespace ECoS {

LocomotiveManager::LocomotiveManager(Kernel& kernel)
  : Object(kernel, ObjectId::locomotiveManager)
{
  requestView();
  send(queryObjects(m_id, Locomotive::options));
}

bool LocomotiveManager::receiveReply(const Reply& reply)
{
  assert(reply.objectId == m_id);

  if(reply.command == Command::queryObjects)
  {
    for(std::string_view line : reply.lines)
    {
      if(uint16_t id; parseId(line, id) && !objectExists(id))
      {
        addObject(std::make_unique<Locomotive>(m_kernel, id));
      }
    }
    return true;
  }

  return Object::receiveReply(reply);
}

bool LocomotiveManager::receiveEvent(const Event& event)
{
  assert(event.objectId == m_id);

  return Object::receiveEvent(event);
}

}
