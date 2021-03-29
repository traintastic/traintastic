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

using nlohmann::json;

WorldSaver::WorldSaver(const World& world)
{
  m_states = json::object();
  json data = json::object();
  json state = json::object();

  data["uuid"] = state["uuid"] = to_string(world.m_uuid);

  data[world.name.name()] = world.name.toJSON();
  data[world.scale.name()] = world.scale.toJSON();

  {
    json objects = json::array();

    for(auto& it : world.m_objects)
      if(ObjectPtr object = it.second.lock())
        objects.push_back(saveObject(object));

    data["objects"] = objects;
    state["states"] = m_states;
  }

  saveToDisk(data, world.m_filename);
  saveToDisk(state, world.m_filenameState);
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
