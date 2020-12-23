/**
 * server/src/world/worldloader.cpp
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

WorldLoader::WorldLoader(const std::filesystem::path& filename) :
  m_world{World::create()}
{
  std::ifstream file(filename);
  if(!file.is_open())
    throw std::runtime_error("can't open " + filename.string());

  json data = json::parse(file);;

  m_world->m_filename = filename;
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
  loadObject(*objectData.object, objectData.json);
  objectData.loaded = true;
}

void WorldLoader::loadObject(Object& object, const json& data)
{
  if(AbstractObjectList* list = dynamic_cast<AbstractObjectList*>(&object))
  {
    json objects = data.value("objects", json::array());
    std::vector<ObjectPtr> items;
    items.reserve(objects.size());
    for(auto& [_, id] : objects.items())
      if(ObjectPtr item = getObject(id))
        items.emplace_back(std::move(item));
    list->setItems(items);
  }
  else if(Board* board = dynamic_cast<Board*>(&object))
  {
    json objects = data.value("tiles", json::array());
    std::vector<ObjectPtr> items;
    board->m_tiles.reserve(objects.size());
    for(auto& [_, id] : objects.items())
      if(auto tile = std::dynamic_pointer_cast<Tile>(getObject(id)))
      {
        if(tile->data().width() > 1 || tile->data().height() > 1)
        {
          const int16_t x2 = tile->location().x + tile->data().width();
          const int16_t y2 = tile->location().y + tile->data().height();
          for(int16_t x = tile->location().x; x < x2; x++)
            for(int16_t y = tile->location().y; y < y2; y++)
              board->m_tiles.emplace(TileLocation{x, y}, tile);
        }
        else
        {
          const TileLocation l = tile->location();
          board->m_tiles.emplace(l, std::move(tile));
        }
      }
  }

  for(auto& [name, value] : data.items())
    if(AbstractProperty* property = object.getProperty(name))
      if(property->type() == ValueType::Object)
      {
        if(contains(property->flags(), PropertyFlags::SubObject))
        {
          loadObject(*property->toObject(), value);
        }
        else
        {
          if(value.is_string())
            property->load(getObject(value));
          else if(value.is_null())
            property->load(ObjectPtr());
        }
      }
      else
        property->load(value);

  //objectData.object->loaded();
}

/*
std::shared_ptr<Object> WorldLoader::getObject(std::string_view id)
{

}
*/
