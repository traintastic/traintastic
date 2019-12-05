/**
 * server/src/core/worldlist.hpp
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

#ifndef SERVER_CORE_WORLDLIST_HPP
#define SERVER_CORE_WORLDLIST_HPP

#include "object.hpp"
#include "table.hpp"
#include "stdfilesystem.hpp"
//#include <memory>
#include <vector>
#include <boost/uuid/uuid.hpp>

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
    const std::filesystem::path m_path;
    std::vector<WorldInfo> m_items;
    std::vector<WorldListTableModel*> m_models;

  public:
    CLASS_ID("world_list");

    WorldList(const std::filesystem::path& path);

    const WorldInfo* find(const boost::uuids::uuid& uuid);

    void buildIndex();

    TableModelPtr getModel() final;
};

#endif
