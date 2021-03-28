/**
 * server/src/world/worldloader.cpp
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

#include "worldloader.hpp"
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/uuid/string_generator.hpp>
#include "world.hpp"
#include "../utils/string.hpp"

#include "../board/board.hpp"
#include "../board/tile/tiles.hpp"
#include "../hardware/commandstation/commandstations.hpp"
#include "../hardware/controller/controllers.hpp"
#include "../hardware/decoder/decoder.hpp"
#include "../hardware/decoder/decoderfunction.hpp"
#include "../hardware/input/inputs.hpp"
#include "../hardware/output/outputs.hpp"
#include "../vehicle/rail/railvehicles.hpp"
#ifndef DISABLE_LUA_SCRIPTING
  #include "../lua/script.hpp"
#endif

using nlohmann::json;

WorldLoader::WorldLoader(const std::filesystem::path& path) :
  m_world{World::create()}
{
  m_world->m_filename = path / World::filename;
  m_world->m_filenameState = path / World::filenameState;

  std::ifstream file(m_world->m_filename);
  if(!file.is_open())
    throw std::runtime_error("can't open " + m_world->m_filename .string());

  json data = json::parse(file);

  m_world->m_uuid = boost::uuids::string_generator()(std::string(data["uuid"]));
  m_world->name = data[m_world->name.name()];

  // create a list of all objects
  for(json object : data["objects"])
  {
    if(auto it = object.find("id"); it != object.end())
      m_objects.insert({it.value().get<std::string>(), {object, nullptr, false}});
    else
      throw std::runtime_error("id missing");
  }

  // then create all objects
  for(auto& it : m_objects)
    if(!it.second.object)
      createObject(it.second);

  // and load their data
  for(auto& it : m_objects)
    if(!it.second.loaded)
      loadObject(it.second);

  // and load their state data
  {
    std::ifstream stateFile(m_world->m_filenameState);
    if(stateFile.is_open())
    {
      json state = json::parse(stateFile);
      if(state["uuid"] == data["uuid"])
        for(auto& [id, values] : state["states"].items())
          if(ObjectPtr object = getObject(id))
            for(auto& [name, value] : values.items())
              if(AbstractProperty* property = object->getProperty(name))
                property->load(value);
    }
  }

  // and finally notify loading is completed
  for(auto& it : m_objects)
  {
    assert(it.second.object);
    it.second.object->loaded();
  }
}

ObjectPtr WorldLoader::getObject(std::string_view id)
{
  std::vector<std::string> ids;
  boost::split(ids, id, [](char c){ return c == '.'; });
  auto itId = ids.cbegin();

  ObjectPtr obj;
  if(auto it = m_objects.find(*itId); it != m_objects.end())
  {
    if(!it->second.object)
      createObject(it->second);
    obj = it->second.object;
  }

  while(obj && ++itId != ids.cend())
  {
    AbstractProperty* property = obj->getProperty(*itId);
    if(property && property->type() == ValueType::Object)
      obj = property->toObject();
    else
      obj = nullptr;
  }

  return obj;
}

void WorldLoader::createObject(ObjectData& objectData)
{
  assert(!objectData.object);

  std::string_view classId = objectData.json["class_id"];
  std::string_view id = objectData.json["id"];

  if(startsWith(classId, CommandStations::classIdPrefix))
    objectData.object = CommandStations::create(m_world, classId, id);
  else if(startsWith(classId, Controllers::classIdPrefix))
    objectData.object = Controllers::create(m_world, classId, id);
  else if(classId == Decoder::classId)
    objectData.object = Decoder::create(m_world, id);
  else if(classId == DecoderFunction::classId)
  {
    const std::string_view decoderId = objectData.json["decoder"];
    if(std::shared_ptr<Decoder> decoder = std::dynamic_pointer_cast<Decoder>(getObject(decoderId)))
      objectData.object = DecoderFunction::create(*decoder, id);
  }
  else if(startsWith(classId, Inputs::classIdPrefix))
    objectData.object = Inputs::create(m_world, classId, id);
  else if(startsWith(classId, Outputs::classIdPrefix))
    objectData.object = Outputs::create(m_world, classId, id);
  else if(classId == Board::classId)
    objectData.object = Board::create(m_world, id);
  else if(startsWith(classId, Tiles::classIdPrefix))
  {
    auto tile = Tiles::create(m_world, classId, id);
    tile->m_location.x = objectData.json["x"];
    tile->m_location.y = objectData.json["y"];
    tile->m_data.setRotate(fromDeg(objectData.json["rotate"]));
    tile->m_data.setSize(objectData.json.value("width", 1), objectData.json.value("height", 1));
    objectData.object = tile;
  }
  else if(startsWith(classId, RailVehicles::classIdPrefix))
    objectData.object = RailVehicles::create(m_world, classId, id);
#ifndef DISABLE_LUA_SCRIPTING
  else if(classId == Lua::Script::classId)
    objectData.object = Lua::Script::create(m_world, id);
#endif
  else
    assert(false);

  if(!objectData.object)
    {};//m_objects.insert(id, object);
}

void WorldLoader::loadObject(ObjectData& objectData)
{
  /*assert*/if(!objectData.object)return;
  assert(!objectData.loaded);
  objectData.object->load(*this, objectData.json);
  objectData.loaded = true;
}

