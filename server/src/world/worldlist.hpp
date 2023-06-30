/**
 * server/src/world/worldlist.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_WORLD_WORLDLIST_HPP
#define TRAINTASTIC_SERVER_WORLD_WORLDLIST_HPP

#include "../core/object.hpp"
#include "../core/table.hpp"
#include <traintastic/utils/stdfilesystem.hpp>
#include <vector>
#include <boost/uuid/uuid.hpp>

class World;
class WorldListTableModel;

class WorldList : public Object, public Table
{
  friend class WorldListTableModel;

  public:
    struct WorldInfo
    {
      boost::uuids::uuid uuid;
      std::string name;
      std::filesystem::path path;

      bool operator >(const WorldInfo& that) const
      {
        return name.compare(that.name) > 0;
      }
    };

  protected:
    static bool readInfo(const nlohmann::json& world, WorldInfo& info);

    const std::filesystem::path m_path;
    std::vector<WorldInfo> m_items;
    std::vector<WorldListTableModel*> m_models;

  public:
    CLASS_ID("world_list");

    static constexpr std::string_view id = classId;

    WorldList(const std::filesystem::path& path);

    std::string getObjectId() const final { return std::string(id); }

    const WorldInfo* find(const boost::uuids::uuid& uuid);

    void buildIndex();

    void update(World& world, const std::filesystem::path& path);

    TableModelPtr getModel() final;
};

#endif
