/**
 * server/src/world/world.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2025 Reinder Feenstra
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

#include <boost/algorithm/string.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "worldsaver.hpp"

#include "../log/log.hpp"
#include "../log/logmessageexception.hpp"
#include "../utils/datetimestr.hpp"
#include "../utils/displayname.hpp"
#include "../traintastic/traintastic.hpp"

#include "../core/method.tpp"
#include "../core/objectproperty.tpp"
#include "../core/objectvectorproperty.tpp"
#include "../core/objectlisttablemodel.hpp"
#include "../core/attributes.hpp"
#include "../core/abstractvectorproperty.hpp"
#include "../core/controllerlist.hpp"

#include "../hardware/input/input.hpp"
#include "../hardware/input/monitor/inputmonitor.hpp"
#include "../hardware/input/list/inputlist.hpp"
#include "../hardware/identification/identification.hpp"
#include "../hardware/identification/list/identificationlist.hpp"
#include "../hardware/output/keyboard/outputkeyboard.hpp"
#include "../hardware/output/list/outputlist.hpp"
#include "../hardware/interface/interfacelist.hpp"
#include "../hardware/decoder/list/decoderlist.hpp"
#include "../hardware/programming/lncv/lncvprogrammer.hpp"
#include "../hardware/programming/lncv/lncvprogrammingcontroller.hpp"

#include "../clock/clock.hpp"

#include "../board/board.hpp"
#include "../board/boardlist.hpp"
#include "../board/list/blockrailtilelist.hpp"
#include "../board/list/linkrailtilelist.hpp"
#include "../board/nx/nxmanager.hpp"
#include "../board/tile/rail/nxbuttonrailtile.hpp"

#include "../zone/zone.hpp"
#include "../zone/zonelist.hpp"

#include "../throttle/list/throttlelist.hpp"
#include "../train/train.hpp"
#include "../train/trainlist.hpp"
#include "../vehicle/rail/railvehiclelist.hpp"
#include "../lua/scriptlist.hpp"
#include "../status/simulationstatus.hpp"
#include "../utils/category.hpp"

using nlohmann::json;

constexpr auto decoderListColumns = DecoderListColumn::Id | DecoderListColumn::Name | DecoderListColumn::Interface | DecoderListColumn::Protocol | DecoderListColumn::Address;
constexpr auto inputListColumns = InputListColumn::Interface | InputListColumn::Channel | InputListColumn::Address;
constexpr auto outputListColumns = OutputListColumn::Interface | OutputListColumn::Channel | OutputListColumn::Address;
constexpr auto identificationListColumns = IdentificationListColumn::Id | IdentificationListColumn::Name | IdentificationListColumn::Interface /*| IdentificationListColumn::Channel*/ | IdentificationListColumn::Address;
constexpr auto throttleListColumns = ThrottleListColumn::Name | ThrottleListColumn::Train | ThrottleListColumn::Interface;

template<class T>
inline static void deleteAll(T& objectList)
{
  while(!objectList.empty())
  {
    if constexpr(std::is_same_v<T, TrainList>)
    {
      if(objectList.front()->active)
      {
        objectList.front()->emergencyStop = true;
        objectList.front()->active = false;
      }
    }
    if constexpr(std::is_same_v<T, ThrottleList>)
    {
      auto& throttle = objectList[0];
      throttle->destroy();
      objectList.removeObject(throttle);
    }
    else
    {
      objectList.delete_(objectList.front());
    }
  }
}

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
  world.identificationControllers.setValueInternal(std::make_shared<ControllerList<IdentificationController>>(world, world.identificationControllers.name()));
  world.lncvProgrammingControllers.setValueInternal(std::make_shared<ControllerList<LNCVProgrammingController>>(world, world.lncvProgrammingControllers.name()));

  world.interfaces.setValueInternal(std::make_shared<InterfaceList>(world, world.interfaces.name()));
  world.decoders.setValueInternal(std::make_shared<DecoderList>(world, world.decoders.name(), decoderListColumns));
  world.inputs.setValueInternal(std::make_shared<InputList>(world, world.inputs.name(), inputListColumns));
  world.outputs.setValueInternal(std::make_shared<OutputList>(world, world.outputs.name(), outputListColumns));
  world.identifications.setValueInternal(std::make_shared<IdentificationList>(world, world.outputs.name(), identificationListColumns));
  world.boards.setValueInternal(std::make_shared<BoardList>(world, world.boards.name()));
  world.zones.setValueInternal(std::make_shared<ZoneList>(world, world.zones.name()));
  world.clock.setValueInternal(std::make_shared<Clock>(world, world.clock.name()));
  world.throttles.setValueInternal(std::make_shared<ThrottleList>(world, world.throttles.name(), throttleListColumns));
  world.trains.setValueInternal(std::make_shared<TrainList>(world, world.trains.name()));
  world.railVehicles.setValueInternal(std::make_shared<RailVehicleList>(world, world.railVehicles.name()));
  world.luaScripts.setValueInternal(std::make_shared<Lua::ScriptList>(world, world.luaScripts.name()));

  world.blockRailTiles.setValueInternal(std::make_shared<BlockRailTileList>(world, world.blockRailTiles.name()));
  world.linkRailTiles.setValueInternal(std::make_shared<LinkRailTileList>(world, world.linkRailTiles.name()));
  world.nxManager.setValueInternal(std::make_shared<NXManager>(world, world.nxManager.name()));

  world.simulationStatus.setValueInternal(std::make_shared<SimulationStatus>(world, world.simulationStatus.name()));
}

World::World(Private /*unused*/) :
  uuid{this, "uuid", to_string(boost::uuids::random_generator()()), PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly},
  name{this, "name", "", PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly},
  scale{this, "scale", WorldScale::H0, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly, [this](WorldScale /*value*/){ updateScaleRatio(); }},
  scaleRatio{this, "scale_ratio", 87, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly},
  onlineWhenLoaded{this, "online_when_loaded", false, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript},
  powerOnWhenLoaded{this, "power_on_when_loaded", false, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript,
    [this](bool value)
    {
      if(!value)
      {
        runWhenLoaded = false; // can't run without power
      }
    }},
  runWhenLoaded{this, "run_when_loaded", false, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript,
    [this](bool value)
    {
      if(value)
      {
        powerOnWhenLoaded = true; // can't run without power
      }
    }},
  correctOutputPosWhenLocked{this, "correct_output_pos_when_locked", true, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript},
  extOutputChangeAction{this, "ext_output_change_action", ExternalOutputChangeAction::EmergencyStopTrain, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript},
  pathReleaseDelay{this, "path_release_delay", 5000, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript},
  debugBlockEvents{this, "debug_block_events", false, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript},
  debugTrainEvents{this, "debug_train_events", false, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript},
  debugZoneEvents{this, "debug_zone_events", false, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript},
  decoderControllers{this, "input_controllers", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore},
  inputControllers{this, "input_controllers", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore},
  outputControllers{this, "output_controllers", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore},
  identificationControllers{this, "identification_controllers", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore},
  lncvProgrammingControllers{this, "lncv_programming_controllers", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore},
  interfaces{this, "interfaces", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore},
  decoders{this, "decoders", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore},
  inputs{this, "inputs", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore},
  outputs{this, "outputs", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore},
  identifications{this, "identifications", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore},
  boards{this, "boards", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly},
  zones{this, "zones", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly},
  clock{this, "clock", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::Store | PropertyFlags::ScriptReadOnly},
  throttles{this, "throttles", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore},
  trains{this, "trains", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly},
  railVehicles{this, "rail_vehicles", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly},
  luaScripts{this, "lua_scripts", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore},
  blockRailTiles{this, "block_rail_tiles", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore},
  linkRailTiles{this, "link_rail_tiles", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore},
  nxManager{this, "nx_manager", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject | PropertyFlags::NoStore},
  statuses(*this, "statuses", {}, PropertyFlags::ReadOnly | PropertyFlags::Store),
  hardwareThrottles{this, "hardware_throttles", 0, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::NoScript},
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
      event(WorldEvent::Offline);
    }},
  online{*this, "online",
    [this]()
    {
      event(WorldEvent::Online);
    }},
  powerOff{*this, "power_off", MethodFlags::ScriptCallable,
    [this]()
    {
      event(WorldEvent::PowerOff);
    }},
  powerOn{*this, "power_on",
    [this]()
    {
      event(WorldEvent::PowerOn);
    }},
  run{*this, "run",
    [this]()
    {
      event(WorldEvent::Run);
    }},
  stop{*this, "stop", MethodFlags::ScriptCallable,
    [this]()
    {
      event(WorldEvent::Stop);
    }},
  mute{this, "mute", false, PropertyFlags::ReadWrite | PropertyFlags::NoStore,
    [this](bool value)
    {
      event(value ? WorldEvent::Mute : WorldEvent::Unmute);
    }},
  noSmoke{this, "no_smoke", false, PropertyFlags::ReadWrite | PropertyFlags::NoStore,
    [this](bool value)
    {
      event(value ? WorldEvent::NoSmoke : WorldEvent::Smoke);
    }},
  simulation{this, "simulation", false, PropertyFlags::ReadWrite | PropertyFlags::NoStore,
    [this](bool value)
    {
      simulationStatus->enabled.setValueInternal(value);
      if(value)
      {
        statuses.appendInternal(simulationStatus.value());
      }
      else
      {
        statuses.removeInternal(simulationStatus.value());
      }
      event(value ? WorldEvent::SimulationEnabled : WorldEvent::SimulationDisabled);
    }},
  simulationStatus{this, "simulation_status", nullptr, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::Internal},
  save{*this, "save", MethodFlags::NoScript,
    [this]()
    {
      try
      {
        // backup world:
        const std::filesystem::path worldDir = Traintastic::instance->worldDir();
        const std::filesystem::path worldBackupDir = Traintastic::instance->worldBackupDir();

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
        {
          Traintastic::instance->settings->lastWorld = uuid.value();
          Traintastic::instance->worldList->update(*this, savePath);
        }

        Log::log(*this, LogMessage::N1022_SAVED_WORLD_X, name.value());
      }
      catch(const std::exception& e)
      {
        Log::log(*this, LogMessage::C1005_SAVING_WORLD_FAILED_X, e);
      }
    }}
  , getObject_{*this, "get_object", MethodFlags::Internal | MethodFlags::ScriptCallable,
      [this](const std::string& objectId)
      {
        return getObjectById(objectId);
      }}
  , getLNCVProgrammer{*this, "get_lncv_programmer", MethodFlags::NoScript,
      [](const ObjectPtr& interface) -> std::shared_ptr<LNCVProgrammer>
      {
        if(auto controller = std::dynamic_pointer_cast<LNCVProgrammingController>(interface))
          return std::make_shared<LNCVProgrammer>(*controller);
        return {};
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

  m_interfaceItems.add(onlineWhenLoaded);
  m_interfaceItems.add(powerOnWhenLoaded);
  m_interfaceItems.add(runWhenLoaded);

  Attributes::addCategory(correctOutputPosWhenLocked, Category::trains);
  Attributes::addEnabled(correctOutputPosWhenLocked, true);
  m_interfaceItems.add(correctOutputPosWhenLocked);

  Attributes::addCategory(extOutputChangeAction, Category::trains);
  Attributes::addEnabled(extOutputChangeAction, true);
  Attributes::addValues(extOutputChangeAction, extOutputChangeActionValues);
  m_interfaceItems.add(extOutputChangeAction);

  Attributes::addCategory(pathReleaseDelay, Category::trains);
  Attributes::addEnabled(pathReleaseDelay, true);
  Attributes::addMinMax(pathReleaseDelay, {0, 15000}); // Up to 15 seconds
  m_interfaceItems.add(pathReleaseDelay);

  // Debug options:
  Attributes::addCategory(debugBlockEvents, Category::debug);
  m_interfaceItems.add(debugBlockEvents);
  Attributes::addCategory(debugTrainEvents, Category::debug);
  m_interfaceItems.add(debugTrainEvents);
  Attributes::addCategory(debugZoneEvents, Category::debug);
  m_interfaceItems.add(debugZoneEvents);

  Attributes::addObjectEditor(decoderControllers, false);
  m_interfaceItems.add(decoderControllers);
  Attributes::addObjectEditor(inputControllers, false);
  m_interfaceItems.add(inputControllers);
  Attributes::addObjectEditor(outputControllers, false);
  m_interfaceItems.add(outputControllers);
  Attributes::addObjectEditor(identificationControllers, false);
  m_interfaceItems.add(identificationControllers);
  Attributes::addObjectEditor(lncvProgrammingControllers, false);
  m_interfaceItems.add(lncvProgrammingControllers);

  Attributes::addObjectEditor(interfaces, false);
  m_interfaceItems.add(interfaces);
  Attributes::addObjectEditor(decoders, false);
  m_interfaceItems.add(decoders);
  Attributes::addObjectEditor(inputs, false);
  m_interfaceItems.add(inputs);
  Attributes::addObjectEditor(outputs, false);
  m_interfaceItems.add(outputs);
  Attributes::addObjectEditor(identifications, false);
  m_interfaceItems.add(identifications);
  Attributes::addObjectEditor(throttles, false);
  m_interfaceItems.add(throttles);
  Attributes::addObjectEditor(boards, false);
  m_interfaceItems.add(boards);

  Attributes::addObjectEditor(zones, false);
  m_interfaceItems.add(zones);

  Attributes::addObjectEditor(clock, false);
  m_interfaceItems.add(clock);
  Attributes::addObjectEditor(trains, false);
  m_interfaceItems.add(trains);
  Attributes::addObjectEditor(railVehicles, false);
  m_interfaceItems.add(railVehicles);
  Attributes::addObjectEditor(luaScripts, false);
  m_interfaceItems.add(luaScripts);

  Attributes::addObjectEditor(blockRailTiles, false);
  m_interfaceItems.add(blockRailTiles);

  Attributes::addObjectEditor(linkRailTiles, false);
  m_interfaceItems.add(linkRailTiles);
  Attributes::addObjectEditor(nxManager, false);
  m_interfaceItems.add(nxManager);

  Attributes::addObjectEditor(statuses, false);
  m_interfaceItems.add(statuses);

  Attributes::addObjectEditor(hardwareThrottles, false);
  m_interfaceItems.add(hardwareThrottles);

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
  Attributes::addEnabled(simulation, false);
  Attributes::addObjectEditor(simulation, false);
  m_interfaceItems.add(simulation);

  m_interfaceItems.add(simulationStatus);

  Attributes::addObjectEditor(save, false);
  m_interfaceItems.add(save);

  m_interfaceItems.add(getObject_);

  Attributes::addObjectEditor(getLNCVProgrammer, false);
  m_interfaceItems.add(getLNCVProgrammer);

  m_interfaceItems.add(onEvent);

  updateEnabled();
}

World::~World()
{
  luaScripts->stopAll(); // no surprise event actions during destruction

  deleteAll(*interfaces);
  deleteAll(*identifications);
  deleteAll(*boards);
  deleteAll(*zones);
  deleteAll(*throttles);
  deleteAll(*trains);
  deleteAll(*railVehicles);
  deleteAll(*luaScripts);
  luaScripts.setValueInternal(nullptr);
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

ObjectPtr World::getObjectById(const std::string& _id) const
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

  ObjectPtr obj = getObjectById(*it);
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

void World::export_(std::vector<std::byte>& data)
{
  try
  {
    WorldSaver saver(*this, data);
    Log::log(*this, LogMessage::N1025_EXPORTED_WORLD_SUCCESSFULLY);
    //return true;
  }
  catch(const std::exception& e)
  {
    throw LogMessageException(LogMessage::C1010_EXPORTING_WORLD_FAILED_X, e);
  }
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
  // Update state:
  switch(value)
  {
    case WorldEvent::EditDisabled:
      state.setValueInternal(state.value() - WorldState::Edit);
      break;

    case WorldEvent::EditEnabled:
      state.setValueInternal(state.value() + WorldState::Edit);
      break;

    case WorldEvent::Offline:
      Log::log(*this, LogMessage::N1013_COMMUNICATION_DISABLED);
      state.setValueInternal(state.value() - WorldState::Online);
      break;

    case WorldEvent::Online:
      Log::log(*this, LogMessage::N1012_COMMUNICATION_ENABLED);
      state.setValueInternal(state.value() + WorldState::Online);
      break;

    case WorldEvent::PowerOff:
      Log::log(*this, LogMessage::N1015_POWER_OFF);
      state.setValueInternal(state.value() - WorldState::PowerOn - WorldState::Run);
      break;

    case WorldEvent::PowerOn:
      Log::log(*this, LogMessage::N1014_POWER_ON);
      state.setValueInternal(state.value() + WorldState::PowerOn);
      break;

    case WorldEvent::Stop:
      Log::log(*this, LogMessage::N1017_STOPPED);
      state.setValueInternal(state.value() - WorldState::Run);
      break;

    case WorldEvent::Run:
      Log::log(*this, LogMessage::N1016_RUNNING);
      state.setValueInternal(state.value() + WorldState::PowerOn + WorldState::Run);
      break;

    case WorldEvent::Unmute:
      Log::log(*this, LogMessage::N1019_MUTE_DISABLED);
      state.setValueInternal(state.value() - WorldState::Mute);
      break;

    case WorldEvent::Mute:
      Log::log(*this, LogMessage::N1018_MUTE_ENABLED);
      state.setValueInternal(state.value() + WorldState::Mute);
      break;

    case WorldEvent::NoSmoke:
      Log::log(*this, LogMessage::N1021_SMOKE_DISABLED);
      state.setValueInternal(state.value() + WorldState::NoSmoke);
      break;

    case WorldEvent::Smoke:
      Log::log(*this, LogMessage::N1020_SMOKE_ENABLED);
      state.setValueInternal(state.value() - WorldState::NoSmoke);
      break;

    case WorldEvent::SimulationDisabled:
      Log::log(*this, LogMessage::N1023_SIMULATION_DISABLED);
      state.setValueInternal(state.value() - WorldState::Simulation);
      break;

    case WorldEvent::SimulationEnabled:
      Log::log(*this, LogMessage::N1024_SIMULATION_ENABLED);
      state.setValueInternal(state.value() + WorldState::Simulation);
      break;
  }

  updateEnabled();

  const WorldState worldState = state;
  worldEvent(worldState, value);
  for(auto& it : m_objects)
    it.second.lock()->worldEvent(worldState, value);
}

void World::updateEnabled()
{
  const bool isOnline = contains(state.value(), WorldState::Online);
  const bool isPoweredOn = contains(state.value(), WorldState::PowerOn);
  const bool isRunning = contains(state.value(), WorldState::Run);

  Attributes::setEnabled(simulation, !isOnline && !isPoweredOn && !isRunning);
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
