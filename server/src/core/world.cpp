/**
 * server/src/core/world.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019 Reinder Feenstra
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
#include <nlohmann/json.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "traintastic.hpp"



#include "../hardware/commandstation/usbxpressnetinterface.hpp"
#include "../hardware/commandstation/z21.hpp"
#include "../core/objectlisttablemodel.hpp"



using nlohmann::json;

const std::string World::id{World::classId};

std::shared_ptr<World> World::create()
{
  auto world = std::make_shared<World>();
  init(world);
  return world;
}

std::shared_ptr<World> World::load(const std::filesystem::path& filename)
{
  auto world = std::make_shared<World>(filename);
  init(world);
  world->load();
  return world;
}

void World::init(const std::shared_ptr<World>& world)
{
  world->commandStations = CommandStationList::create(world, "command_station_list");
  world->decoders = DecoderList::create(world, "decoder_list");
}

World::World() :
  Object(),
  m_uuid{boost::uuids::random_generator()()},
  name{this, "name", "", PropertyFlags::AccessWCC},
  commandStations{this, "command_stations", nullptr, PropertyFlags::AccessRRR},
  decoders{this, "decoders", nullptr, PropertyFlags::AccessRRR}
{
}

World::World(const std::filesystem::path& filename) :
  World()
{
  m_filename = filename;
}

std::string World::getUniqueId(const std::string& prefix) const
{
  std::string id;
  uint32_t number = 0;
  do
  {
    id = prefix + "_" + std::to_string(++number);
  }
  while(m_objects.find(id) != m_objects.end());

  return id;
}

bool World::isObject(const std::string& _id) const
{
  return m_objects.find(_id) != m_objects.end();
}

ObjectPtr World::getObject(const std::string& _id) const
{
  auto it = m_objects.find(_id);
  if(it != m_objects.end())
    return it->second.lock();
  else
    return ObjectPtr();
}

void World::load()
{
  std::ifstream file(m_filename);
  if(file.is_open())
  {
    json world = json::parse(file);
    m_uuid = boost::uuids::string_generator()(std::string(world["uuid"]));
    name = world[name.name()];











    std::weak_ptr<World> w = shared_ptr<World>();

#if 0
    auto cs = Hardware::CommandStation::USBXpressNetInterface::create(w, "cs1");
#else
    auto cs = Hardware::CommandStation::Z21::create(w, "cs1");
    //cs->hostname = "192.168.1.2";
    cs->hostname = "192.168.16.254";
#endif
    commandStations->add(cs);


    {
      auto dec = Hardware::Decoder::create(w, "dec_acts6701");
      decoders->add(dec);
      dec->name = "ACTS 6701";
      dec->commandStation = cs;
      dec->protocol = DecoderProtocol::DCC;
      dec->address = 1;
      dec->speedSteps = 126;
    }
    {
      auto dec = Hardware::Decoder::create(w, "dec_acts6703");
      decoders->add(dec);
      dec->name = "ACTS 6703";
      dec->commandStation = cs;
      dec->protocol = DecoderProtocol::DCC;
      dec->address = 3;
      dec->speedSteps = 126;
    }
    {
      auto dec = Hardware::Decoder::create(w, "dec_acts6705");
      decoders->add(dec);
      dec->name = "ACTS 6705";
      dec->commandStation = cs;
      dec->protocol = DecoderProtocol::DCC;
      dec->address = 5;
      dec->speedSteps = 126;
    }
    {
      auto dec = Hardware::Decoder::create(w, "dec_g2000");
      decoders->add(dec);
      dec->name = "G2000";
      dec->commandStation = cs;
      dec->protocol = DecoderProtocol::DCC;
      dec->address = 3302;
      dec->longAddress = true;
      dec->speedSteps = 126;

      {
        auto f = Hardware::DecoderFunction::create(w, "dec_g2000_f0");
        f->number = 0;
        dec->functions->add(f);
        f->m_decoder = dec.get();
      }

      {
        auto f = Hardware::DecoderFunction::create(w, "dec_g2000_f2");
        f->number = 2;
        f->momentary = true;
        dec->functions->add(f);
        f->m_decoder = dec.get();
      }
    }

    //cs->online = true;













    Traintastic::instance->console->notice(id, "Loaded world " + name.value());
  }
  else
    throw std::runtime_error("Can't open file");
}

void World::save()
{
  json objects = json::array();
  for(auto& it : m_objects)
    if(ObjectPtr object = it.second.lock())
    {
      json objectData;
      objectData["class_id"] = object->getClassId();
      for(auto& item : object->interfaceItems())
        if(AbstractProperty* property = dynamic_cast<AbstractProperty*>(&item.second))
        {
          if(property->type() == PropertyType::Object)
          {
            if(IdObject* idObject = dynamic_cast<IdObject*>(property->toObject().get()))
              objectData[property->name()] = idObject->id.toJSON();
            else
              objectData[property->name()] = nullptr;
          }
          else
            objectData[property->name()] = property->toJSON();
        }

      objects.push_back(objectData);
    }

  json world;
  world["uuid"] = to_string(m_uuid);
  world[name.name()] = name.value();
  world["objects"] = objects;

  std::ofstream file(m_filename);
  if(file.is_open())
  {
    file << world.dump(2);
    Traintastic::instance->console->notice(id, "Saved world " + name.value());
  }
  else
    Traintastic::instance->console->critical(id, "Can't write to world file");
}
