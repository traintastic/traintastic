/**
 * server/src/world/worldloader.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2023 Reinder Feenstra
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
#include "../core/isvalidobjectid.hpp"
#include "../utils/startswith.hpp"
#include "../utils/stripsuffix.hpp"
#include "ctwreader.hpp"
#include "../log/logmessageexception.hpp"
#include <version.hpp>

#include "../board/board.hpp"
#include "../board/tile/tiles.hpp"
#include "../hardware/interface/interfaces.hpp"
#include "../hardware/decoder/decoder.hpp"
#include "../hardware/decoder/decoderfunction.hpp"
#include "../hardware/identification/identification.hpp"
#include "../vehicle/rail/railvehicles.hpp"
#include "../train/train.hpp"
#include "../lua/script.hpp"

using nlohmann::json;

WorldLoader::WorldLoader()
  : m_world{World::create()}
{
}

WorldLoader::WorldLoader(std::filesystem::path path)
  : WorldLoader()
{
  if(path.extension() == World::dotCTW)
    m_ctw = std::make_unique<CTWReader>(path);
  else
    m_path = std::move(path);

  load();
}

WorldLoader::WorldLoader(const std::vector<std::byte>& memory)
  : WorldLoader()
{
  m_ctw = std::make_unique<CTWReader>(memory);

  load();
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

void WorldLoader::load()
{
  m_states = json::object();

  json data;

  // load file(s):
  if(m_ctw)
  {
    if(!m_ctw->readFile(World::filename, data))
      throw std::runtime_error(std::string("can't read ").append(World::filename));

    json state;
    if(m_ctw->readFile(World::filenameState, state) && state["uuid"] == data["uuid"])
        m_states = state["states"];
  }
  else
  {
    std::ifstream file(m_path / World::filename);
    if(!file.is_open())
      throw std::runtime_error("can't open " + (m_path / World::filename).string());
    data = json::parse(file);

    std::ifstream stateFile(m_path / World::filenameState);
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

  // check version:
  //! \todo require this for >= v0.2
  {
    json traintastic = data["traintastic"];
    if(traintastic.is_object())
    {
      json version = traintastic["version"];
      if(version.is_object())
      {
        const uint16_t major = version["major"].get<uint16_t>();
        const uint16_t minor = version["minor"].get<uint16_t>();
        const uint16_t patch = version["patch"].get<uint16_t>();

        if((major != TRAINTASTIC_VERSION_MAJOR) ||
            (minor > TRAINTASTIC_VERSION_MINOR) ||
            (minor == TRAINTASTIC_VERSION_MINOR && patch > TRAINTASTIC_VERSION_PATCH))
        {
          throw LogMessageException(LogMessage::C1013_CANT_LOAD_WORLD_SAVED_WITH_NEWER_VERSION_REQUIRES_AT_LEAST_X,
              std::string("Traintastic server v").append(std::to_string(major)).append(".").append(std::to_string(minor)).append(".").append(std::to_string(patch)));
        }
      }
    }
  }

  // create a list of all objects
  m_objects.insert({m_world->getObjectId(), {data, m_world, false}});
  for(json object : data["objects"])
  {
    if(auto it = object.find("id"); it != object.end())
    {
      auto id = it.value().get<std::string>();
      if(!isValidObjectId(id))
        throw std::runtime_error("invalid object id value");
      m_objects.insert({std::move(id), {object, nullptr, false}});
    }
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
    it.second.object->loaded();
}

void WorldLoader::createObject(ObjectData& objectData)
{
  assert(!objectData.object);

  std::string_view classId = objectData.json["class_id"].get<std::string_view>();
  std::string_view id = objectData.json["id"].get<std::string_view>();

  if(startsWith(classId, Interfaces::classIdPrefix))
    objectData.object = Interfaces::create(*m_world, classId, id);
  else if(classId == Decoder::classId)
    objectData.object = Decoder::create(*m_world, id);
  else if(classId == Input::classId)
    objectData.object = Input::create(*m_world, id);
  else if(classId == Output::classId)
    objectData.object = Output::create(*m_world, id);
  else if(classId == Identification::classId)
    objectData.object = Identification::create(*m_world, id);
  else if(classId == Board::classId)
    objectData.object = Board::create(*m_world, id);
  else if(startsWith(classId, Tiles::classIdPrefix))
  {
    if(auto tile = Tiles::create(*m_world, classId, id))
    {
      // x, y, width, height are read in Board::load()
      tile->x.setValueInternal(objectData.json["x"]);
      tile->y.setValueInternal(objectData.json["y"]);
      tile->height.setValueInternal(objectData.json.value("height", 1));
      tile->width.setValueInternal(objectData.json.value("width", 1));
      objectData.object = tile;
    }
  }
  else if(startsWith(classId, RailVehicles::classIdPrefix))
  {
    if(classId == "vehicle.rail.freight_car") { classId = FreightWagon::classId; } //! \todo Remove in v0.4
    objectData.object = RailVehicles::create(*m_world, classId, id);
  }
  else if(classId == Train::classId)
    objectData.object = Train::create(*m_world, id);
  else if(classId == Lua::Script::classId)
    objectData.object = Lua::Script::create(*m_world, id);

  if(!objectData.object)
    throw LogMessageException(LogMessage::C1012_UNKNOWN_CLASS_X_CANT_RECREATE_OBJECT_X, classId, id);
}

void WorldLoader::loadObject(ObjectData& objectData)
{
  assert(objectData.object);
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
