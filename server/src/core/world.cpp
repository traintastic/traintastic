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
#include "../hardware/commandstation/commandstationlist.hpp"
#include "../hardware/decoder/decoderlist.hpp"

using nlohmann::json;

const std::string World::id{World::classId};

World::World() :
  Object(),
  m_uuid{boost::uuids::random_generator()()},
  name{this, "name", "", PropertyFlags::AccessWCC},
  commandStationList{std::make_shared<CommandStationList>("command_station_list")},
  decoderList{std::make_shared<DecoderList>("decoder_list")}
{
}

World::World(const std::filesystem::path& filename) :
  World()
{
  m_filename = filename;
  load();
}

void World::load()
{
  std::ifstream file(m_filename);
  if(file.is_open())
  {
    json world = json::parse(file);
    m_uuid = boost::uuids::string_generator()(std::string(world["uuid"]));
    name = world[name.name()];
    Traintastic::instance->console->notice(id, "Loaded world " + name.value());
  }
  else
    throw std::runtime_error("Can't open file");
}

void World::save()
{
  json world;
  world["uuid"] = to_string(m_uuid);
  world[name.name()] = name.value();

  std::ofstream file(m_filename);
  if(file.is_open())
  {
    file << world.dump(2);
    Traintastic::instance->console->notice(id, "Saved world " + name.value());
  }
  else
    Traintastic::instance->console->critical(id, "Can't write to world file");
}
