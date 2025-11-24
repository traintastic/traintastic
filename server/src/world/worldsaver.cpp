/**
 * server/src/world/worldsaver.cpp
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

#include "worldsaver.hpp"
#include <fstream>
#include <boost/uuid/uuid_io.hpp>
#include <version.hpp>
#include "world.hpp"
#include "../core/stateobject.hpp"
#include "../core/objectproperty.tpp"
#include "../status/simulationstatus.hpp"
#include "../utils/sha1.hpp"
#include "ctwwriter.hpp"

using nlohmann::json;

WorldSaver::WorldSaver(const World& world)
{
  m_states = json::object();
  m_data = json::object();
  m_state = json::object();

  {
    json worldState = json::object();
    world.Object::save(*this, m_data, worldState);
    if(!worldState.empty())
      m_states[world.getObjectId()] = worldState;
    m_data.erase("class_id");
    // ugly fixup: remove simulation status object
    if(auto it = m_data.find(world.statuses.name()); it != m_data.end() && it->is_array()) [[likely]]
    {
      const auto removeId = world.simulationStatus->getObjectId();
      for(size_t i = 0; i < it->size(); i++)
      {
        if(it->operator[](i) == removeId)
        {
          it->erase(i);
          break;
        }
      }
    }
  }

  m_data["uuid"] = m_state["uuid"] = world.uuid.value();

  // traintastic version info:
  {
    json version;
    version["major"] = TRAINTASTIC_VERSION_MAJOR;
    version["minor"] = TRAINTASTIC_VERSION_MINOR;
    version["patch"] = TRAINTASTIC_VERSION_PATCH;
    version["version"] = TRAINTASTIC_VERSION_FULL;

    json traintastic;
    traintastic["version"] = version;

    m_data["traintastic"] = m_state["traintastic"] = traintastic;
  }

  {
    json objects = json::array();
    json stateObjects = json::array();

    for(const auto& it : world.m_objects)
    {
      if(ObjectPtr object = it.second.lock())
      {
        if(auto stateObject = std::dynamic_pointer_cast<StateObject>(object))
        {
          json data = saveStateObject(stateObject);
          if(!data.empty())
            stateObjects.push_back(std::move(data));
        }
        else
        {
          json data = saveObject(object);
          if(!data.empty())
            objects.push_back(std::move(data));
        }
      }
    }

    std::sort(objects.begin(), objects.end(),
      [](const json& a, const json& b)
      {
        return (a["id"] < b["id"]);
      });

    m_data["objects"] = objects;
    m_state["objects"] = stateObjects;
    m_state["states"] = m_states;
  }
}

WorldSaver::WorldSaver(const World& world, const std::filesystem::path& path)
  : WorldSaver(world)
{
  if(path.extension() == World::dotCTW)
  {
    CTWWriter ctw(path);
    writeCTW(ctw);
  }
  else
  {
    saveToDisk(m_data, path / World::filename);
    saveToDisk(m_state, path / World::filenameState);
    deleteFiles(path);
    writeFiles(path);
  }
}

WorldSaver::WorldSaver(const World& world, std::vector<std::byte>& memory)
  : WorldSaver(world)
{
  CTWWriter ctw(memory);
  writeCTW(ctw);
}

void WorldSaver::writeCTW(CTWWriter& ctw)
{
  ctw.writeFile(World::filename, m_data);
  ctw.writeFile(World::filenameState, m_state);
  for(const auto& file : m_writeFiles)
    ctw.writeFile(file.first, file.second);
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

json WorldSaver::saveStateObject(const std::shared_ptr<StateObject>& object)
{
  json objectState = json::object();
  static_cast<Object&>(*object).save(*this, objectState, objectState);
  return objectState;
}

void WorldSaver::deleteFile(std::filesystem::path filename)
{
  m_deleteFiles.emplace_back(std::move(filename));
}

void WorldSaver::writeFile(std::filesystem::path filename, std::string data)
{
  m_writeFiles.push_back({std::move(filename), std::move(data)});
}

void WorldSaver::deleteFiles(const std::filesystem::path& basePath)
{
  for(const auto& filename : m_deleteFiles)
  {
    std::error_code ec;
    std::filesystem::remove(basePath / filename, ec);
    //! \todo report error if removal fails of existing file
  }
}

void WorldSaver::writeFiles(const std::filesystem::path& basePath)
{
  for(const auto& file : m_writeFiles)
    saveToDisk(file.second, basePath / file.first);
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
