/**
 * server/src/hardware/protocol/ecos/object/feedbackmanager.cpp
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

#include "feedbackmanager.hpp"
#include <cassert>
#include "feedback.hpp"
#include "../messages.hpp"

namespace ECoS {

FeedbackManager::FeedbackManager(Kernel& kernel)
  : Object(kernel, ObjectId::feedbackManager)
{
  requestView();
  send(queryObjects(m_id, Feedback::options));
}

bool FeedbackManager::receiveReply(const Reply& reply)
{
  assert(reply.objectId == m_id);

  if(reply.command == Command::queryObjects)
  {
    for(std::string_view line : reply.lines)
    {
      Line data;
      if(parseLine(line, data) && !objectExists(data.objectId))
        addObject(std::make_unique<Feedback>(m_kernel, data));
    }
    return true;
  }

  return Object::receiveReply(reply);
}

bool FeedbackManager::receiveEvent(const Event& event)
{
  assert(event.objectId == m_id);

  return Object::receiveEvent(event);
}

}
