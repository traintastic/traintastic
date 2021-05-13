/**
 * server/src/hardware/controller/controller.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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

#include "controller.hpp"
#include "controllerlist.hpp"
#include "controllerlisttablemodel.hpp"
#include "../commandstation/commandstation.hpp"
#include "../../world/getworld.hpp"
#include "../../core/attributes.hpp"

Controller::Controller(const std::weak_ptr<World>& _world, std::string_view _id) :
  IdObject(_world, _id),
  name{this, "name", "", PropertyFlags::ReadWrite | PropertyFlags::Store},
  commandStation{this, "command_station", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](const std::shared_ptr<CommandStation>& value)
    {
      std::shared_ptr<Controller> controller = shared_ptr<Controller>();
      assert(controller);

      if(commandStation)
        commandStation->controllers->removeObject(controller);

      if(value)
        value->controllers->addObject(controller);

      return true;
    }},
  active{this, "active", false, PropertyFlags::ReadWrite | PropertyFlags::StoreState, nullptr,
    std::bind(&Controller::setActive, this, std::placeholders::_1)},
  notes{this, "notes", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  auto world = _world.lock();
  const bool editable = world && contains(world->state.value(), WorldState::Edit);

  Attributes::addDisplayName(name, "object:name");
  m_interfaceItems.add(name);
  Attributes::addEnabled(commandStation, editable);
  Attributes::addObjectList(commandStation, world->commandStations);
  m_interfaceItems.add(commandStation);
  m_interfaceItems.add(active);
  Attributes::addDisplayName(notes, "object:notes");
  m_interfaceItems.add(notes);
}

void Controller::addToWorld()
{
  IdObject::addToWorld();

  if(auto world = m_world.lock())
    world->controllers->addObject(shared_ptr<Controller>());
}

void Controller::worldEvent(WorldState state, WorldEvent event)
{
  IdObject::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  commandStation.setAttributeEnabled(editable);

  try
  {
    switch(event)
    {
      case WorldEvent::Offline:
        active = false;
        break;

      case WorldEvent::Online:
        active = true;
        break;

      default:
        break;
    }
  }
  catch(...)
  {
  }
}
