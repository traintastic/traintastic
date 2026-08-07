/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2019-2026 Reinder Feenstra
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
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "../traintastic/traintastic.hpp"
#include "../log/log.hpp"
#include "worldlisttablemodel.hpp"
#include "ctwreader.hpp"
#include "ctwmodifier.hpp"
#include "libarchiveerror.hpp"
#include "../core/method.tpp"
#include "../utils/readfile.hpp"
#include "../utils/writefile.hpp"

using nlohmann::json;

WorldList::WorldList(const std::filesystem::path& path)
  : m_path{path}
  , duplicate(*this, "duplicate",
      [this](const std::string& srcUUID, const std::string& newName)
      {
        boost::uuids::uuid uuid = boost::uuids::nil_generator()();
        try
        {
          uuid = boost::uuids::string_generator()(srcUUID);
        }
        catch(const std::exception&)
        {
          Log::log(*this, LogMessage::E1001_INVALID_WORLD_UUID_X, srcUUID);
          return false;
        }

        const auto* info = find(uuid);
        if(!info)
        {
          Log::log(*this, LogMessage::E1002_WORLD_X_DOESNT_EXIST, to_string(uuid));
          return false;
        }

        const auto newUUID = boost::uuids::random_generator()();

        auto patchWorld =
          [&newUUID, &newName](nlohmann::json& worldData, nlohmann::json& worldState)
          {
            worldData["uuid"] = to_string(newUUID);
            worldData["name"] = newName;

            worldState["uuid"] = worldData["uuid"];
          };

        if(std::filesystem::is_regular_file(info->path))
        {
          try
          {
            CTWModifier ctw(info->path);

            nlohmann::json worldData;
            nlohmann::json worldState;
            if(!ctw.readFile(World::filename, worldData) || !ctw.readFile(World::filenameState, worldState))
            {
              return false;
            }

            patchWorld(worldData, worldState);

            const auto newFile = (m_path / to_string(newUUID)) += World::dotCTW;

            if(!ctw.updateFile(World::filename, worldData) ||
                !ctw.updateFile(World::filenameState, worldState) ||
                !ctw.save(newFile))
            {
              return false;
            }

            add({newUUID, newName, newFile});

            return true;
          }
          catch(const std::exception& e)
          {
            return false;
          }
        }
        else if(std::filesystem::is_directory(info->path))
        {
          const auto newDir = m_path / to_string(newUUID);
          std::error_code ec;
          std::filesystem::copy(info->path, newDir, std::filesystem::copy_options::recursive, ec);
          if(ec)
          {
            return false;
          }

          auto worldData = readFileJSON(newDir / World::filename);
          auto worldState = readFileJSON(newDir / World::filenameState);
          if(!worldData || !worldState)
          {
            std::filesystem::remove_all(newDir, ec); // remove dir on fail
            return false;
          }

          patchWorld(*worldData, *worldState);

          if(!writeFileJSON(newDir / World::filename, *worldData) || !writeFileJSON(newDir / World::filenameState, *worldState))
          {
            std::filesystem::remove_all(newDir, ec); // remove dir on fail
            return false;
          }

          add({newUUID, newName, newDir});

          return true;
        }

        return false;
      })
  , delete_{*this, "delete",
      [this](const std::string& worldUUID)
      {
        boost::uuids::uuid uuid = boost::uuids::nil_generator()();
        try
        {
          uuid = boost::uuids::string_generator()(worldUUID);
        }
        catch(const std::exception&)
        {
          Log::log(*this, LogMessage::E1001_INVALID_WORLD_UUID_X, worldUUID);
          return false;
        }

        auto it = findItem(uuid);
        if(it == m_items.end())
        {
          Log::log(*this, LogMessage::E1002_WORLD_X_DOESNT_EXIST, to_string(uuid));
          return false;
        }

        if(std::filesystem::is_regular_file(it->path))
        {
          std::error_code ec;
          std::filesystem::remove(it->path, ec);
          if(ec)
          {
            // TODO: Log::log(*this, LogMessage::E100x_File deletion failed
            return false;
          }
        }
        else if(std::filesystem::is_directory(it->path))
        {
          std::error_code ec;
          std::filesystem::remove_all(it->path, ec);
          if(ec)
          {
            // TODO: Log::log(*this, LogMessage::E100x_DIRECTORY deletion failed
            return false;
          }
        }
        else
        {
          return false;
        }

        m_items.erase(it);
        itemsChanged();
        return true;
      }}
{
  m_interfaceItems.add(duplicate);
  m_interfaceItems.add(delete_);

  if(!std::filesystem::is_directory(m_path))
    std::filesystem::create_directories(m_path);

  buildIndex();
}

const WorldList::WorldInfo* WorldList::find(const boost::uuids::uuid& uuid)
{
  if(auto it = findItem(uuid); it != m_items.end())
  {
    return &*it;
  }
  return nullptr;
}

void WorldList::buildIndex()
{
  Log::log(Traintastic::classId, LogMessage::I1005_BUILDING_WORLD_INDEX);

  m_items.clear();

  WorldInfo info;
  for(const auto& it : std::filesystem::directory_iterator(m_path))
  {
    info.path = it.path();

    if(info.path.extension() == World::dotCTW)
    {
      try
      {
        CTWReader ctw(info.path);

        json world;
        if(ctw.readFile(World::filename, world) && readInfo(world, info))
          m_items.push_back(info);
      }
      catch(const LibArchiveError& e)
      {
        Log::log(Traintastic::classId, LogMessage::W1003_READING_WORLD_X_FAILED_LIBARCHIVE_ERROR_X_X, info.path.filename(), e.errorCode, e.what());
      }
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
          Log::log(Traintastic::classId, LogMessage::C1004_READING_WORLD_FAILED_X_X, e, worldFile);
        }
      }
    }
  }

  sort();
}

void WorldList::add(WorldInfo info)
{
  m_items.emplace_back(std::move(info));
  sort();
  itemsChanged();
}

void WorldList::update(World& world, const std::filesystem::path& path)
{
  const auto uuid = boost::uuids::string_generator()(world.uuid.value());

  if(auto it = findItem(uuid); it != m_items.end())
  {
    it->name = world.name;
    it->path = path;
  }
  else // new world
  {
    m_items.emplace_back(WorldInfo{uuid, world.name.value(), path});
  }
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

WorldList::Items::iterator WorldList::findItem(const boost::uuids::uuid& uuid)
{
  return std::find_if(m_items.begin(), m_items.end(),
    [&uuid](const auto& item)
    {
      return item.uuid == uuid;
    });
}

void WorldList::itemsChanged()
{
  const auto rowCount = static_cast<uint32_t>(m_items.size());
  for(auto* model : m_models)
  {
    model->setRowCount(rowCount);
    model->rowsChanged(0, rowCount - 1);
  }
}

void WorldList::sort()
{
  std::sort(m_items.begin(), m_items.end(),
    [](const WorldInfo& a, const WorldInfo& b) -> bool
    {
      return a > b;
    });
}
