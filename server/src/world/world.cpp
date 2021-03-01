/**
 * server/src/world/world.cpp
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

#include "world.hpp"
#include <fstream>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "worldsaver.hpp"
#include "../core/traintastic.hpp"
#include "../core/objectlisttablemodel.hpp"
#include "../core/attributes.hpp"

using nlohmann::json;

std::shared_ptr<World> World::create()
{
  auto world = std::make_shared<World>(Private());
  init(world);
  return world;
}

void World::init(const std::shared_ptr<World>& world)
{
  world->commandStations.setValueInternal(std::make_shared<CommandStationList>(*world, world->commandStations.name()));
  world->decoders.setValueInternal(std::make_shared<DecoderList>(*world, world->decoders.name()));
  world->inputs.setValueInternal(std::make_shared<InputList>(*world, world->inputs.name()));
  world->outputs.setValueInternal(std::make_shared<OutputList>(*world, world->outputs.name()));
  world->controllers.setValueInternal(std::make_shared<ControllerList>(*world, world->controllers.name()));
  world->loconets.setValueInternal(std::make_shared<LocoNetList>(*world, world->loconets.name()));
  world->xpressnets.setValueInternal(std::make_shared<XpressNetList>(*world, world->xpressnets.name()));
  world->boards.setValueInternal(std::make_shared<BoardList>(*world, world->boards.name()));
  world->clock.setValueInternal(std::make_shared<Clock>(*world, world->clock.name()));
  world->trains.setValueInternal(std::make_shared<TrainList>(*world, world->trains.name()));
  world->railVehicles.setValueInternal(std::make_shared<RailVehicleList>(*world, world->railVehicles.name()));
#ifndef DISABLE_LUA_SCRIPTING
  world->luaScripts.setValueInternal(std::make_shared<Lua::ScriptList>(*world, world->luaScripts.name()));
#endif
}

World::World(Private) :
  Object(),
  m_uuid{boost::uuids::random_generator()()},
  name{this, "name", "", PropertyFlags::ReadWrite},
  scale{this, "scale", WorldScale::H0, PropertyFlags::ReadWrite},
  commandStations{this, "command_stations", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject},
  decoders{this, "decoders", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject},
  inputs{this, "inputs", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject},
  outputs{this, "outputs", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject},
  controllers{this, "controllers", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject},
  loconets{this, "loconets", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject},
  xpressnets{this, "xpressnets", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject},
  boards{this, "boards", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject},
  clock{this, "clock", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject},
  trains{this, "trains", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject},
  railVehicles{this, "rail_vehicles", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject},
#ifndef DISABLE_LUA_SCRIPTING
  luaScripts{this, "lua_scripts", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject},
#endif
  state{this, "state", WorldState::TrackPowerOff, PropertyFlags::ReadOnly},
  emergencyStop{*this, "emergency_stop",
    [this]()
    {
      Traintastic::instance->console->notice(classId, "Emergency stop");
      event(WorldEvent::EmergencyStop);
    }},
  trackPowerOff{*this, "track_power_off",
    [this]()
    {
      Traintastic::instance->console->notice(classId, "Track power: off");
      state.setValueInternal(state.value() + WorldState::TrackPowerOff);
      event(WorldEvent::TrackPowerOff);
    }},
  trackPowerOn{*this, "track_power_on",
    [this]()
    {
      Traintastic::instance->console->notice(classId, "Track power: on");
      state.setValueInternal(state.value() - WorldState::TrackPowerOff);
      event(WorldEvent::TrackPowerOn);
    }},
  edit{this, "edit", false, PropertyFlags::ReadWrite,
    [this](bool value)
    {
      if(value)
      {
        Traintastic::instance->console->notice(classId, "Edit mode: enabled");
        state.setValueInternal(state.value() + WorldState::Edit);
        event(WorldEvent::EditEnabled);
      }
      else
      {
        Traintastic::instance->console->notice(classId, "Edit mode: disabled");
        state.setValueInternal(state.value() - WorldState::Edit);
        event(WorldEvent::EditDisabled);
      }
    }},
  save{*this, "save",
    [this]()
    {
      try
      {
        WorldSaver saver(*this);
        Traintastic::instance->console->notice(classId, "Saved world " + name.value());
      }
      catch(const std::exception& e)
      {
        Traintastic::instance->console->critical(classId, std::string("Saving world failed: ").append(e.what()));
      }
    }}
{
  m_filename = Traintastic::instance->worldDir() / to_string(m_uuid) / filename;
  m_filenameState = Traintastic::instance->worldDir() / to_string(m_uuid) / filenameState;

  m_interfaceItems.add(name);
  Attributes::addValues(scale, WorldScaleValues);
  m_interfaceItems.add(scale);

  m_interfaceItems.add(commandStations);
  m_interfaceItems.add(decoders);
  m_interfaceItems.add(inputs);
  m_interfaceItems.add(outputs);
  m_interfaceItems.add(controllers);
  m_interfaceItems.add(loconets);
  m_interfaceItems.add(xpressnets);
  m_interfaceItems.add(boards);
  m_interfaceItems.add(clock);
  m_interfaceItems.add(trains);
  m_interfaceItems.add(railVehicles);
#ifndef DISABLE_LUA_SCRIPTING
  m_interfaceItems.add(luaScripts);
#endif

  m_interfaceItems.add(state);
  m_interfaceItems.add(emergencyStop);
  m_interfaceItems.add(trackPowerOff);
  m_interfaceItems.add(trackPowerOn);
  m_interfaceItems.add(edit);

  m_interfaceItems.add(save);
}

std::string World::getUniqueId(std::string_view prefix) const
{
  std::string id{prefix};
  id.append("_");
  uint32_t number = 0;
  do
  {
    id.resize(prefix.size() + 1);
    id.append(std::to_string(++number));
  }
  while(isObject(id));

  return id;
}

bool World::isObject(const std::string& _id) const
{
  return m_objects.find(_id) != m_objects.end() || _id == id || _id == Traintastic::id;
}

ObjectPtr World::getObject(const std::string& _id) const
{
  auto it = m_objects.find(_id);
  if(it != m_objects.end())
    return it->second.lock();
  else if(_id == classId)
    return std::const_pointer_cast<Object>(shared_from_this());
  else
    return ObjectPtr();
}

void World::event(WorldEvent event)
{
  const WorldState st = state;
  worldEvent(st, event);
  for(auto& it : m_objects)
    it.second.lock()->worldEvent(st, event);
}
