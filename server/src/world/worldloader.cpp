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
#include <boost/uuid/uuid_io.hpp>
#include "world.hpp"
#include "../utils/startswith.hpp"
#include "ctwreader.hpp"

#include "../board/board.hpp"
#include "../board/tile/tiles.hpp"
#include "../hardware/interface/interfaces.hpp"
#include "../hardware/decoder/decoder.hpp"
#include "../hardware/decoder/decoderfunction.hpp"
#include "../vehicle/rail/railvehicles.hpp"
#include "../train/train.hpp"
#ifndef DISABLE_LUA_SCRIPTING
  #include "../lua/script.hpp"
#endif

using nlohmann::json;

WorldLoader::WorldLoader(std::filesystem::path path) :
  m_path{std::move(path)},
  m_world{World::create()}
{
  m_states = json::object();

  json data;

  // load file(s):
  if(m_path.extension() == World::dotCTW)
  {
    m_ctw = std::make_unique<CTWReader>(m_path);

    if(!m_ctw->readFile(World::filename, data))
      throw std::runtime_error(std::string("can't read ").append(World::filename));

    json state;
    if(m_ctw->readFile(World::filenameState, state) && state["uuid"] == data["uuid"])
        m_states = state["states"];
  }
  else
  {
    std::ifstream file(path / World::filename);
    if(!file.is_open())
      throw std::runtime_error("can't open " + (path / World::filename).string());
    data = json::parse(file);

    std::ifstream stateFile(path / World::filenameState);
    if(stateFile.is_open())
    {
      json state = json::parse(stateFile);
      if(state["uuid"] == data["uuid"])
        m_states = state["states"];
      else
      {} /// @todo log warning
    }
  }

  // check if UUID is valid:
  m_world->uuid.setValueInternal(to_string(boost::uuids::string_generator()(std::string(data["uuid"]))));

  // create a list of all objects
  m_objects.insert({m_world->getObjectId(), {data, m_world, false}});
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

  // and load their data/state
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

WorldLoader::~WorldLoader() = default; // default here, so we can use a forward declaration of CTWReader in the header.

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

json WorldLoader::getState(const std::string& id) const
{
  return m_states.value(id, json::object());
}

void WorldLoader::createObject(ObjectData& objectData)
{
  assert(!objectData.object);

  std::string_view classId = objectData.json["class_id"];
  std::string_view id = objectData.json["id"];

  if(startsWith(classId, Interfaces::classIdPrefix))
    objectData.object = Interfaces::create(m_world, classId, id);
  else if(classId == Decoder::classId)
    objectData.object = Decoder::create(m_world, id);
  else if(classId == DecoderFunction::classId)
  {
    // backwards compatibility < 0.1
    const std::string_view decoderId = objectData.json["decoder"];
    if(std::shared_ptr<Decoder> decoder = std::dynamic_pointer_cast<Decoder>(getObject(decoderId)))
    {
      auto f = std::make_shared<DecoderFunction>(*decoder, objectData.json["number"]);
      objectData.object = std::static_pointer_cast<Object>(f);
      decoder->functions->items.appendInternal(f);
    }
  }
  else if(classId == Input::classId)
    objectData.object = Input::create(m_world, id);
  else if(classId == Output::classId)
    objectData.object = Output::create(m_world, id);
  else if(classId == Board::classId)
    objectData.object = Board::create(m_world, id);
  else if(startsWith(classId, Tiles::classIdPrefix))
  {
    auto tile = Tiles::create(m_world, classId, id);

    // x, y, width, height are read in Board::load()
    tile->x.setValueInternal(objectData.json["x"]);
    tile->y.setValueInternal(objectData.json["y"]);
    tile->height.setValueInternal(objectData.json.value("height", 1));
    tile->width.setValueInternal(objectData.json.value("width", 1));

    // backwards compatibility < 0.1
    if(objectData.json["rotate"].is_number_integer())
    {
      tile->rotate.setValueInternal(fromDeg(objectData.json["rotate"]));
      objectData.json.erase("rotate");
    }

    objectData.object = tile;
  }
  else if(startsWith(classId, RailVehicles::classIdPrefix))
    objectData.object = RailVehicles::create(m_world, classId, id);
  else if(classId == Train::classId)
    objectData.object = Train::create(m_world, id);
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

bool WorldLoader::readFile(const std::filesystem::path& filename, std::string& data)
{
  if(m_ctw)
  {
    if(!m_ctw->readFile(filename, data))
      return false;
  }
  else
  {
    std::ifstream file(m_path / filename, std::ios::in | std::ios::binary | std::ios::ate);
    if(!file.is_open())
      return false;
    const size_t size = file.tellg();
    data.resize(size);
    file.seekg(std::ios::beg);
    file.read(data.data(), size);
  }
  return true;
}
