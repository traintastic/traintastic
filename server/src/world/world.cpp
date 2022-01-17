/**
 * server/src/world/world.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022 Reinder Feenstra
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
#include <iomanip>
#include <boost/algorithm/string.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "worldsaver.hpp"
#include "../core/traintastic.hpp"
#include "../core/objectlisttablemodel.hpp"
#include "../core/attributes.hpp"
#include "../core/abstractvectorproperty.hpp"
#include "../log/log.hpp"
#include "../utils/displayname.hpp"
#include "../core/traintastic.hpp"

using nlohmann::json;

std::shared_ptr<World> World::create()
{
  auto world = std::make_shared<World>(Private());
  init(*world);
  return world;
}

void World::init(World& world)
{
  world.decoderControllers.setValueInternal(std::make_shared<ControllerList<DecoderController>>(world, world.decoderControllers.name()));
  world.inputControllers.setValueInternal(std::make_shared<ControllerList<InputController>>(world, world.inputControllers.name()));
  world.outputControllers.setValueInternal(std::make_shared<ControllerList<OutputController>>(world, world.outputControllers.name()));

  world.interfaces.setValueInternal(std::make_shared<InterfaceList>(world, world.interfaces.name()));
  world.decoders.setValueInternal(std::make_shared<DecoderList>(world, world.decoders.name()));
  world.inputs.setValueInternal(std::make_shared<InputList>(world, world.inputs.name()));
  world.outputs.setValueInternal(std::make_shared<OutputList>(world, world.outputs.name()));
  world.boards.setValueInternal(std::make_shared<BoardList>(world, world.boards.name()));
  world.clock.setValueInternal(std::make_shared<Clock>(world, world.clock.name()));
  world.trains.setValueInternal(std::make_shared<TrainList>(world, world.trains.name()));
  world.railVehicles.setValueInternal(std::make_shared<RailVehicleList>(world, world.railVehicles.name()));
#ifndef DISABLE_LUA_SCRIPTING
  world.luaScripts.setValueInternal(std::make_shared<Lua::ScriptList>(world, world.luaScripts.name()));
#endif
}

World::World(Private /*unused*/) :
  uuid{this, "uuid", to_string(boost::uuids::random_generator()()), PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly},
  name{this, "name", "", PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly},
  scale{this, "scale", WorldScale::H0, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly, [this](WorldScale /*value*/){ updateScaleRatio(); }},
  scaleRatio{this, "scale_ratio", 87, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly},
  decoderControllers{this, "input_controllers", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore},
  inputControllers{this, "input_controllers", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore},
  outputControllers{this, "output_controllers", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore},
  interfaces{this, "interfaces", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore},
  decoders{this, "decoders", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore},
  inputs{this, "inputs", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore},
  outputs{this, "outputs", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore},
  boards{this, "boards", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly},
  clock{this, "clock", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore},
  trains{this, "trains", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly},
  railVehicles{this, "rail_vehicles", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly},
#ifndef DISABLE_LUA_SCRIPTING
  luaScripts{this, "lua_scripts", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore},
#endif
  state{this, "state", WorldState(), PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly},
  edit{this, "edit", false, PropertyFlags::ReadWrite | PropertyFlags::NoStore,
    [this](bool value)
    {
      if(value)
      {
        Log::log(*this, LogMessage::N1010_EDIT_MODE_ENABLED);
        state.setValueInternal(state.value() + WorldState::Edit);
        event(WorldEvent::EditEnabled);
      }
      else
      {
        Log::log(*this, LogMessage::N1011_EDIT_MODE_DISABLED);
        state.setValueInternal(state.value() - WorldState::Edit);
        event(WorldEvent::EditDisabled);
      }
    }},
  offline{*this, "offline",
    [this]()
    {
      Log::log(*this, LogMessage::N1013_COMMUNICATION_DISABLED);
      state.setValueInternal(state.value() - WorldState::Online);
      event(WorldEvent::Offline);
    }},
  online{*this, "online",
    [this]()
    {
      Log::log(*this, LogMessage::N1012_COMMUNICATION_ENABLED);
      state.setValueInternal(state.value() + WorldState::Online);
      event(WorldEvent::Online);
    }},
  powerOff{*this, "power_off", MethodFlags::ScriptCallable,
    [this]()
    {
      Log::log(*this, LogMessage::N1015_POWER_OFF);
      state.setValueInternal(state.value() - WorldState::PowerOn);
      event(WorldEvent::PowerOff);
    }},
  powerOn{*this, "power_on",
    [this]()
    {
      Log::log(*this, LogMessage::N1014_POWER_ON);
      state.setValueInternal(state.value() + WorldState::PowerOn);
      event(WorldEvent::PowerOn);
    }},
  run{*this, "run",
    [this]()
    {
      Log::log(*this, LogMessage::N1016_RUNNING);
      state.setValueInternal(state.value() + WorldState::Run);
      event(WorldEvent::Run);
    }},
  stop{*this, "stop", MethodFlags::ScriptCallable,
    [this]()
    {
      Log::log(*this, LogMessage::N1017_STOPPED);
      state.setValueInternal(state.value() - WorldState::Run);
      event(WorldEvent::Stop);
    }},
  mute{this, "mute", false, PropertyFlags::ReadWrite | PropertyFlags::NoStore,
    [this](bool value)
    {
      if(value)
      {
        Log::log(*this, LogMessage::N1018_MUTE_ENABLED);
        state.setValueInternal(state.value() + WorldState::Mute);
        event(WorldEvent::Mute);
      }
      else
      {
        Log::log(*this, LogMessage::N1019_MUTE_DISABLED);
        state.setValueInternal(state.value() - WorldState::Mute);
        event(WorldEvent::Unmute);
      }
    }},
  noSmoke{this, "no_smoke", false, PropertyFlags::ReadWrite | PropertyFlags::NoStore,
    [this](bool value)
    {
      if(value)
      {
        Log::log(*this, LogMessage::N1021_SMOKE_DISABLED);
        state.setValueInternal(state.value() + WorldState::NoSmoke);
        event(WorldEvent::NoSmoke);
      }
      else
      {
        Log::log(*this, LogMessage::N1020_SMOKE_ENABLED);
        state.setValueInternal(state.value() - WorldState::NoSmoke);
        event(WorldEvent::Smoke);
      }
    }},
  save{*this, "save", MethodFlags::NoScript,
    [this]()
    {
      try
      {
        // backup world:
        const std::filesystem::path worldDir = Traintastic::instance->worldDir();
        const std::filesystem::path worldBackupDir = Traintastic::instance->worldBackupDir();
        auto dateTimeStr =
          []()
          {
            const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::stringstream ss;
            ss << std::put_time(std::localtime(&now), "_%Y%m%d_%H%M%S");
            return ss.str();
          };

        if(!std::filesystem::is_directory(worldBackupDir))
        {
          std::error_code ec;
          std::filesystem::create_directories(worldBackupDir, ec);
          if(ec)
            Log::log(*this, LogMessage::C1007_CREATING_WORLD_BACKUP_DIRECTORY_FAILED_X, ec);
        }

        if(std::filesystem::is_directory(worldDir / uuid.value()))
        {
          std::error_code ec;
          std::filesystem::rename(worldDir / uuid.value(), worldBackupDir / uuid.value() += dateTimeStr(), ec);
          if(ec)
            Log::log(*this, LogMessage::C1006_CREATING_WORLD_BACKUP_FAILED_X, ec);
        }

        if(std::filesystem::is_regular_file(worldDir / uuid.value() += dotCTW))
        {
          std::error_code ec;
          std::filesystem::rename(worldDir / uuid.value() += dotCTW, worldBackupDir / uuid.value() += dateTimeStr() += dotCTW, ec);
          if(ec)
            Log::log(*this, LogMessage::C1006_CREATING_WORLD_BACKUP_FAILED_X, ec);
        }

        // save world:
        std::filesystem::path savePath = worldDir / uuid.value();
        if(!Traintastic::instance->settings->saveWorldUncompressed)
          savePath += dotCTW;

        WorldSaver saver(*this, savePath);

        if(Traintastic::instance)
          Traintastic::instance->settings->lastWorld = uuid.value();

        Log::log(*this, LogMessage::N1022_SAVED_WORLD_X, name.value());
      }
      catch(const std::exception& e)
      {
        Log::log(*this, LogMessage::C1005_SAVING_WORLD_FAILED_X, e);
      }
    }}
  , onEvent{*this, "on_event", EventFlags::Scriptable}
{
  Attributes::addDisplayName(uuid, DisplayName::World::uuid);
  m_interfaceItems.add(uuid);
  Attributes::addDisplayName(name, DisplayName::Object::name);
  m_interfaceItems.add(name);
  Attributes::addEnabled(scale, false);
  Attributes::addValues(scale, WorldScaleValues);
  m_interfaceItems.add(scale);
  Attributes::addEnabled(scaleRatio, false);
  Attributes::addMinMax(scaleRatio, 1., 1000.);
  Attributes::addVisible(scaleRatio, false);
  m_interfaceItems.add(scaleRatio);

  Attributes::addObjectEditor(decoderControllers, false);
  m_interfaceItems.add(decoderControllers);
  Attributes::addObjectEditor(inputControllers, false);
  m_interfaceItems.add(inputControllers);
  Attributes::addObjectEditor(outputControllers, false);
  m_interfaceItems.add(outputControllers);

  Attributes::addObjectEditor(interfaces, false);
  m_interfaceItems.add(interfaces);
  Attributes::addObjectEditor(decoders, false);
  m_interfaceItems.add(decoders);
  Attributes::addObjectEditor(inputs, false);
  m_interfaceItems.add(inputs);
  Attributes::addObjectEditor(outputs, false);
  m_interfaceItems.add(outputs);
  Attributes::addObjectEditor(boards, false);
  m_interfaceItems.add(boards);
  Attributes::addObjectEditor(clock, false);
  m_interfaceItems.add(clock);
  Attributes::addObjectEditor(trains, false);
  m_interfaceItems.add(trains);
  Attributes::addObjectEditor(railVehicles, false);
  m_interfaceItems.add(railVehicles);
#ifndef DISABLE_LUA_SCRIPTING
  Attributes::addObjectEditor(luaScripts, false);
  m_interfaceItems.add(luaScripts);
#endif

  Attributes::addObjectEditor(state, false);
  m_interfaceItems.add(state);
  Attributes::addObjectEditor(edit, false);
  m_interfaceItems.add(edit);
  Attributes::addObjectEditor(offline, false);
  m_interfaceItems.add(offline);
  Attributes::addObjectEditor(online, false);
  m_interfaceItems.add(online);
  Attributes::addObjectEditor(powerOff, false);
  m_interfaceItems.add(powerOff);
  Attributes::addObjectEditor(powerOn, false);
  m_interfaceItems.add(powerOn);
  Attributes::addObjectEditor(stop, false);
  m_interfaceItems.add(stop);
  Attributes::addObjectEditor(run, false);
  m_interfaceItems.add(run);
  Attributes::addObjectEditor(mute, false);
  m_interfaceItems.add(mute);
  Attributes::addObjectEditor(noSmoke, false);
  m_interfaceItems.add(noSmoke);

  Attributes::addObjectEditor(save, false);
  m_interfaceItems.add(save);

  m_interfaceItems.add(onEvent);
}

std::string World::getUniqueId(std::string_view prefix) const
{
  std::string uniqueId{prefix};
  uniqueId.append("_");
  uint32_t number = 0;
  do
  {
    uniqueId.resize(prefix.size() + 1);
    uniqueId.append(std::to_string(++number));
  }
  while(isObject(uniqueId));

  return uniqueId;
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
  if(_id == classId)
    return std::const_pointer_cast<Object>(shared_from_this());
  return ObjectPtr();
}

ObjectPtr World::getObjectByPath(std::string_view path) const
{
  std::vector<std::string> ids;
  boost::split(ids, path, [](char c){ return c == '.'; });
  auto it = ids.cbegin();

  ObjectPtr obj = getObject(*it);
  while(obj && ++it != ids.cend())
  {
    if(AbstractProperty* property = obj->getProperty(*it); property && property->type() == ValueType::Object)
      obj = property->toObject();
    else if(AbstractVectorProperty* vectorProperty = obj->getVectorProperty(*it); vectorProperty && vectorProperty->type() == ValueType::Object)
    {
      obj.reset();
      const size_t size = vectorProperty->size();
      for(size_t i = 0; i < size; i++)
      {
        ObjectPtr v = vectorProperty->getObject(i);
        if(path == v->getObjectId())
        {
          obj = v;
          it++;
          break;
        }
      }
    }
    else
      obj.reset();
  }
  return obj;
}

void World::loaded()
{
  updateScaleRatio();
  Object::loaded();
}

void World::worldEvent(WorldState worldState, WorldEvent worldEvent)
{
  Object::worldEvent(worldState, worldEvent);

  const bool editState = contains(worldState, WorldState::Edit);
  const bool runState = contains(worldState, WorldState::Run);

  Attributes::setEnabled(scale, editState && !runState);
  Attributes::setEnabled(scaleRatio, editState && !runState);

  fireEvent(onEvent, worldState, worldEvent);
}

void World::event(const WorldEvent value)
{
  const WorldState worldState = state;
  worldEvent(worldState, value);
  for(auto& it : m_objects)
    it.second.lock()->worldEvent(worldState, value);
}

void World::updateScaleRatio()
{
  if(scale != WorldScale::Custom)
  {
    scaleRatio.setValueInternal(getScaleRatio(scale));
    Attributes::setVisible(scaleRatio, false);
  }
  else
    Attributes::setVisible(scaleRatio, true);
}
