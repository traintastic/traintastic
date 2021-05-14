/**
 * server/src/world/worldsaver.cpp
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

#include "worldsaver.hpp"
#include <fstream>
#include <boost/uuid/uuid_io.hpp>
#include "world.hpp"
#include "../utils/sha1.hpp"

using nlohmann::json;

WorldSaver::WorldSaver(const World& world) :
  m_path{std::filesystem::path(world.m_filename).remove_filename()}
{
  m_states = json::object();
  json data = json::object();
  json state = json::object();

  {
    json worldState = json::object();
    world.Object::save(*this, data, worldState);
    if(!worldState.empty())
      m_states[world.getObjectId()] = worldState;
    data.erase("class_id");
  }

  data["uuid"] = state["uuid"] = to_string(world.m_uuid);

  {
    json objects = json::array();

    for(auto& it : world.m_objects)
      if(ObjectPtr object = it.second.lock())
        objects.push_back(saveObject(object));

    std::sort(objects.begin(), objects.end(),
      [](const json& a, const json& b)
      {
        return (a["id"] < b["id"]);
      });

    data["objects"] = objects;
    state["states"] = m_states;
  }

  saveToDisk(data, world.m_filename);
  saveToDisk(state, world.m_filenameState);
  deleteFiles();
  writeFiles();
}

json WorldSaver::saveObject(const ObjectPtr& object)
{
  json objectData = json::object();
  json objectState = json::object();

  object->save(*this, objectData, objectState);

  if(!objectState.empty())
    m_states[object->getObjectId()] = objectState;

  return objectData;
}

void WorldSaver::deleteFile(std::filesystem::path filename)
{
  m_deleteFiles.emplace_back(std::move(filename));
}

void WorldSaver::writeFile(std::filesystem::path filename, std::string data)
{
  m_writeFiles.push_back({std::move(filename), std::move(data)});
}

void WorldSaver::deleteFiles()
{
  for(const auto& filename : m_deleteFiles)
  {
    std::error_code ec;
    std::filesystem::remove(m_path / filename, ec);
    //! \todo report error if removal fails of existing file
  }
}

void WorldSaver::writeFiles()
{
  for(const auto& file : m_writeFiles)
    saveToDisk(file.second, m_path / file.first);
}

void WorldSaver::saveToDisk(const json& data, const std::filesystem::path& filename)
{
  std::filesystem::path dir = std::filesystem::path(filename).remove_filename();
  std::string s = dir.string();
  if(!std::filesystem::is_directory(dir))
    std::filesystem::create_directories(dir);

  std::ofstream file(filename);
  if(file.is_open())
  {
    file << data.dump(2);
    //Traintastic::instance->console->notice(classId, "Saved world " + name.value());
  }
  else
    throw std::runtime_error("file not open");
    //Traintastic::instance->console->critical(classId, "Can't write to world file");
}

void WorldSaver::saveToDisk(const std::string& data, const std::filesystem::path& filename)
{
  if(std::filesystem::exists(filename) &&
      std::filesystem::file_size(filename) == data.size() &&
      Sha1::of(filename) == Sha1::of(data))
    return;

  std::filesystem::path dir = std::filesystem::path(filename).remove_filename();
  std::string s = dir.string();
  if(!std::filesystem::is_directory(dir))
    std::filesystem::create_directories(dir);

  std::ofstream file(filename);
  if(file.is_open())
  {
    file << data;
    //Traintastic::instance->console->notice(classId, "Saved world " + name.value());
  }
  else
    throw std::runtime_error("file not open");
    //Traintastic::instance->console->critical(classId, "Can't write to world file");
}
