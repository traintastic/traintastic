/**
 * server/src/world/worldloader.hpp
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

#ifndef TRAINTASTIC_SERVER_WORLD_WORLDLOADER_HPP
#define TRAINTASTIC_SERVER_WORLD_WORLDLOADER_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "../core/stdfilesystem.hpp"
#include "../core/objectptr.hpp"

class Object;
class World;

class WorldLoader
{
  private:
    struct ObjectData
    {
      nlohmann::json json;
      std::shared_ptr<Object> object;
      bool loaded;
    };

    std::shared_ptr<World> m_world;
    std::unordered_map<std::string, ObjectData> m_objects;

    const ObjectPtr& getObject(std::string_view id);
    void createObject(ObjectData& objectData);
    void loadObject(ObjectData& objectData);
    void loadObject(Object& object, const nlohmann::json& data);

  public:
    WorldLoader(const std::filesystem::path& filename);

    std::shared_ptr<World> world() { return m_world; }
};


#endif
