/**
 * server/src/world/worldsaver.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_WORLD_WORLDSAVER_HPP
#define TRAINTASTIC_SERVER_WORLD_WORLDSAVER_HPP

#include <list>
#include "../core/objectptr.hpp"
#include <traintastic/utils/stdfilesystem.hpp>
#include "../utils/json.hpp"

class World;
class StateObject;
class CTWWriter;

class WorldSaver
{
  private:
    nlohmann::json m_states;
    nlohmann::json m_data;
    nlohmann::json m_state;
    std::list<std::filesystem::path> m_deleteFiles;
    std::list<std::pair<std::filesystem::path, std::string>> m_writeFiles;

    WorldSaver(const World& world);

    void writeCTW(CTWWriter& ctw);

    void deleteFiles(const std::filesystem::path& basePath);
    void writeFiles(const std::filesystem::path& basePath);
    static void saveToDisk(const nlohmann::json& data, const std::filesystem::path& filename);
    static void saveToDisk(const std::string& data, const std::filesystem::path& filename);

  public:
    WorldSaver(const World& world, const std::filesystem::path& path);
    WorldSaver(const World& world, std::vector<std::byte>& memory);

    nlohmann::json saveObject(const ObjectPtr& object);
    nlohmann::json saveStateObject(const std::shared_ptr<StateObject>& object);

    void deleteFile(std::filesystem::path filename);
    void writeFile(std::filesystem::path filename, std::string data);
};

#endif
