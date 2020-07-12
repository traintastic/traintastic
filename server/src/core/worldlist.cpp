/**
 * server/src/core/worldlist.cpp
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
#include <nlohmann/json.hpp>
#include <boost/uuid/string_generator.hpp>
#include "traintastic.hpp"
#include "worldlisttablemodel.hpp"

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
  auto& console = *Traintastic::instance->console;
  console.info(Traintastic::id, "Building world index");

  m_items.clear();

  boost::uuids::string_generator string_to_uuid;
  WorldInfo info;
  for(auto& it : std::filesystem::directory_iterator(m_path))
  {
    info.path = it.path();
    const auto worldFile = info.path / "traintastic.json";
    if(std::filesystem::is_directory(info.path) && std::filesystem::is_regular_file(worldFile))
    {
      std::ifstream file(worldFile);
      if(file.is_open())
      {
        try
        {
          json world = json::parse(file);

          auto it = world.find("uuid");
          if(it == world.end())
            continue;
          info.uuid = string_to_uuid(std::string(*it));

          it = world.find("name");
          if(it == world.end())
            continue;
          info.name = *it;

          if(!info.uuid.is_nil() && !info.name.empty())
            m_items.push_back(info);
        }
        catch(const std::exception& e)
        {
          console.critical(Traintastic::id, std::string(e.what()) + " (" + worldFile.string() + ")");
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
