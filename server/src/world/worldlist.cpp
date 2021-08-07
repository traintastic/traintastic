/**
 * server/src/world/worldlist.cpp
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

#include "worldlist.hpp"
#include <fstream>
#include <boost/uuid/string_generator.hpp>
#include "../core/traintastic.hpp"
#include "../log/log.hpp"
#include "worldlisttablemodel.hpp"
#include "ctwreader.hpp"

using nlohmann::json;

WorldList::WorldList(const std::filesystem::path& path) :
  Object(),
  m_path{path}
{
  if(!std::filesystem::is_directory(m_path))
    std::filesystem::create_directories(m_path);

  buildIndex();
}

const WorldList::WorldInfo* WorldList::find(const boost::uuids::uuid& uuid)
{
  for(const auto& info : m_items)
    if(info.uuid == uuid)
      return &info;

  return nullptr;
}

void WorldList::buildIndex()
{
  Log::log(*Traintastic::instance, LogMessage::I1005_BUILDING_WORLD_INDEX);

  m_items.clear();

  WorldInfo info;
  for(auto& it : std::filesystem::directory_iterator(m_path))
  {
    info.path = it.path();

    if(info.path.extension() == World::dotCTW)
    {
      CTWReader ctw(info.path);

      json world;
      if(ctw.readFile(World::filename, world) && readInfo(world, info))
        m_items.push_back(info);

      continue;
    }

    const auto worldFile = info.path / World::filename;
    if(std::filesystem::is_directory(info.path) && std::filesystem::is_regular_file(worldFile))
    {
      std::ifstream file(worldFile);
      if(file.is_open())
      {
        try
        {
          json world = json::parse(file);

          if(readInfo(world, info))
            m_items.push_back(info);
        }
        catch(const std::exception& e)
        {
          Log::log(*Traintastic::instance, LogMessage::C1004_READING_WORLD_FAILED_X_X, e, worldFile);
        }
      }
    }
  }

  std::sort(m_items.begin(), m_items.end(), [](const WorldInfo& a, const WorldInfo& b) -> bool { return a > b; });

  //for(auto& model : m_models)
  //  model->setRowCount(m_items.size());
}

TableModelPtr WorldList::getModel()
{
  return std::make_shared<WorldListTableModel>(*this);
}

bool WorldList::readInfo(const json& world, WorldInfo& info)
{
  auto it = world.find("uuid");
  if(it == world.end())
    return false;
  info.uuid = boost::uuids::string_generator()(std::string(*it));

  it = world.find("name");
  if(it == world.end())
    return false;
  info.name = *it;

  return !info.uuid.is_nil();
}
