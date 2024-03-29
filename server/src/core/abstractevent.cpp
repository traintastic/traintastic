/**
 * server/src/core/abstractevent.cpp
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

#include "abstractevent.hpp"
#include "abstracteventhandler.hpp"
#include "object.hpp"

AbstractEvent::AbstractEvent(Object& object, std::string_view name, EventFlags flags)
  : InterfaceItem(object, name)
  , m_flags{flags}
{
}

AbstractEvent::~AbstractEvent()
{
  while(!m_handlers.empty())
    m_handlers.front()->disconnect();
}

void AbstractEvent::connect(std::shared_ptr<AbstractEventHandler> handler)
{
  assert(handler);
  assert(&handler->event() == this);
  m_handlers.emplace_back(std::move(handler));
}

bool AbstractEvent::disconnect(const std::shared_ptr<AbstractEventHandler>& handler)
{
  auto it = std::find(m_handlers.begin(), m_handlers.end(), handler);
  if(it != m_handlers.end())
  {
    m_handlers.erase(it);
    return true;
  }
  return false;
}

void AbstractEvent::fire(const Arguments& args)
{
  const auto handlers{m_handlers}; // copy, list can be modified while iterating
  for(const auto& handler : handlers)
    handler->execute(args);

  m_object.onEventFired(*this, args);
}
