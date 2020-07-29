/**
 * server/src/world/world.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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
  world->controllers.setValueInternal(std::make_shared<ControllerList>(*world, world->controllers.name()));
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
  controllers{this, "controllers", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject},
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

  m_interfaceItems.add(name);
  m_interfaceItems.add(scale);

  m_interfaceItems.add(commandStations);
  m_interfaceItems.add(decoders);
  m_interfaceItems.add(inputs);
  m_interfaceItems.add(controllers);
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

std::string World::getUniqueId(const std::string& prefix) const
{
  std::string id;
  uint32_t number = 0;
  do
  {
    id = prefix + "_" + std::to_string(++number);
  }
  while(isObject(id));

  return id;
}

bool World::isObject(const std::string& _id) const
{
  return m_objects.find(_id) != m_objects.end() || _id == classId || _id == Traintastic::id;
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
/*
void World::load()
{
  std::ifstream file(m_filename);
  if(file.is_open())
  {
    json data = json::parse(file);
    m_uuid = boost::uuids::string_generator()(std::string(data["uuid"]));
    name = data[name.name()];

    json objects = data["objects"];












    std::weak_ptr<World> w = shared_ptr<World>();

#if 0
    auto cs = Hardware::CommandStation::Z21::create(w, "cs1");
    //cs->hostname = "192.168.1.2";
    cs->hostname = "192.168.16.254";
/ *
    for(int i = 1; i <= 16; i++)
    {
      auto input = LocoNetInput::create(w, "ln_in_" + std::to_string(i));
      input->loconet = cs->loconet.value();
      input->address = i;
      input->name = "LocoNet input #" + std::to_string(i);
    }
* /

#else
  #if 1
    auto cs = Hardware::CommandStation::LI10x::create(w, "cs1");
    cs->port = "/dev/ttyUSB0";
  #else
    auto cs = Hardware::CommandStation::USBXpressNetInterface::create(w, "cs1");
  #endif
    cs->xpressnet->commandStation = XpressNetCommandStation::Roco10764;
#endif
    commandStations->addObject(cs);


    {
      auto dec = Hardware::Decoder::create(w, "dec_acts6701");
      dec->name = "ACTS 6701";
      dec->commandStation = cs;
      dec->protocol = DecoderProtocol::DCC;
      dec->address = 1;
      dec->speedSteps = 126;
    }
    {
      auto dec = Hardware::Decoder::create(w, "dec_acts6703");
      dec->name = "ACTS 6703";
      dec->commandStation = cs;
      dec->protocol = DecoderProtocol::DCC;
      dec->address = 3;
      dec->speedSteps = 126;
    }
    {
      auto dec = Hardware::Decoder::create(w, "dec_acts6705");
      dec->name = "ACTS 6705";
      dec->commandStation = cs;
      dec->protocol = DecoderProtocol::DCC;
      dec->address = 5;
      dec->speedSteps = 126;
    }
    {
      auto dec = Hardware::Decoder::create(w, "dec_g2000");
      dec->name = "G2000";
      dec->commandStation = cs;
      dec->protocol = DecoderProtocol::DCC;
      dec->address = 3302;
      dec->longAddress = true;
      dec->speedSteps = 126;
/ *
      {
        auto f = Hardware::DecoderFunction::create(*dec, "dec_g2000_f0");
        f->number = 0;
        dec->functions->add(f);
        f->m_decoder = dec.get();
      }

      {
        auto f = Hardware::DecoderFunction::create(*dec, "dec_g2000_f2");
        f->number = 2;
        f->momentary = true;
        dec->functions->add(f);
        f->m_decoder = dec.get();
      }* /
    }
    {
      auto dec = Hardware::Decoder::create(w, "dec_br211");
      dec->name = "BR211";
      dec->commandStation = cs;
      dec->protocol = DecoderProtocol::DCC;
      dec->address = 3311;
      dec->longAddress = true;
      dec->speedSteps = 126;

      for(int i = 0; i <= 20; i++)
        dec->functions->add();
/ *
      {
        auto f = dec->functions->add();
        f->number = 0;
        f->name = "Light";
      }

      {
        auto f = dec->functions->add();
        f->number = 1;
        f->name = "Sound";
      }
      * /
    }

    //cs->online = true;



#ifndef DISABLE_LUA_SCRIPTING
    {
      auto script = Lua::Script::create(w, getUniqueId("script"));
      script->name = "test";
      //script->enabled = true;
      script->code =
        "console.debug(enum)\n"
        "console.debug(enum.world_event)\n"
        "console.debug(enum.world_event.TRACK_POWER_ON)\n"
        ""
        "function init()\n"
        "  console.info(VERSION)\n"
        "  --console.debug(world)\n"
        "  console.notice(world.name)\n"
        "  --console.debug(dec)\n"
        "  --console.debug(dec.functions)\n"
        "  --console.debug(dec.functions.add)\n"
        "  console.debug(dec.functions.add())\n"
        "end\n"
        "\n"
        "function mode_changed(mode)\n"
        "  console.warning(mode)\n"
        "  local f = console.debug(dec.functions.add())\n"
        "  f.name ='test'\n"
        "end\n"
        "\n"
        "function state_changed(state, event)\n"
        "  console.debug(state)\n"
        "  console.debug(event)\n"
        "  console.debug(event == enum.world_event.TRACK_POWER_ON)\n"
        "end\n";
      luaScripts->addObject(script);
    }
#endif

    {
      auto z21app = Hardware::Controller::Z21App::create(w, "z21app");
      z21app->commandStation = cs;
      z21app->active = true;
      controllers->addObject(z21app);
    }




    Traintastic::instance->console->notice(classId, "Loaded world " + name.value());
  }
  else
    throw std::runtime_error("Can't open file");
}
*/
json World::saveObject(const ObjectPtr& object)
{
  json objectData;

  objectData["class_id"] = object->getClassId();

  if(Hardware::DecoderFunction* function = dynamic_cast<Hardware::DecoderFunction*>(object.get()))
    objectData["decoder"] = function->decoder().id.toJSON();

  for(auto& item : object->interfaceItems())
    if(AbstractProperty* property = dynamic_cast<AbstractProperty*>(&item.second))
    {
      if(!property->isStoreable())
        continue;

      if(property->type() == ValueType::Object)
      {
        if(ObjectPtr value = property->toObject())
        {
          if(IdObject* idObject = dynamic_cast<IdObject*>(value.get()))
            objectData[property->name()] = idObject->id.toJSON();
          else if(SubObject* subObject = dynamic_cast<SubObject*>(value.get()))
          {
            if((property->flags() & PropertyFlags::SubObject) == PropertyFlags::SubObject)
              objectData[property->name()] = saveObject(value);
            else
              objectData[property->name()] = subObject->id();
          }
        }
        else
          objectData[property->name()] = nullptr;
      }
      else
        objectData[property->name()] = property->toJSON();
    }

  return objectData;
}
