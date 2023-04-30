/**
 * server/src/traintastic/traintastic.hpp
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

#ifndef TRAINTASTIC_SERVER_TRAINTASTIC_TRAINTASTIC_HPP
#define TRAINTASTIC_SERVER_TRAINTASTIC_TRAINTASTIC_HPP

#include <memory>
#include <list>
#include <traintastic/utils/stdfilesystem.hpp>
#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>
#include "../core/object.hpp"
#include "../core/property.hpp"
#include "../core/objectproperty.hpp"
#include "../core/method.hpp"
#include "settings.hpp"
#include "../world/world.hpp"
#include "../world/worldlist.hpp"

class Server;

class Traintastic final : public Object
{
  public:
    enum RunStatus
    {
      ExitSuccess = 0,
      Restart = 1,
      ExitFailure = 2,
    };

  private:
    bool m_restart;
    const std::filesystem::path m_dataDir;
    std::shared_ptr<Server> m_server;

    boost::asio::signal_set m_signalSet;

    bool start();
    void stop();

    void loadWorldUUID(const boost::uuids::uuid& uuid);
    void loadWorldPath(const std::filesystem::path& path);

    static void signalHandler(const boost::system::error_code& ec, int signalNumber);

  public:
    CLASS_ID("traintastic");

    static constexpr std::string_view id = classId;

    static std::shared_ptr<Traintastic> instance;

    ObjectProperty<Settings> settings;
    Property<std::string> version;
    ObjectProperty<World> world;
    ObjectProperty<WorldList> worldList;
    Method<void()> newWorld;
    Method<void(std::string)> loadWorld;
    Method<void()> closeWorld;
    Method<void()> restart;
    Method<void()> shutdown;

    Traintastic(const std::filesystem::path& dataDir);
    ~Traintastic() final = default;

    std::string getObjectId() const final { return std::string(id); }

    const std::filesystem::path& dataDir() const { return m_dataDir; }
    std::filesystem::path dataBackupDir() const { return m_dataDir / ".backup"; }

    std::filesystem::path worldDir() const { return dataDir() / "world"; }
    std::filesystem::path worldBackupDir() const { return dataBackupDir() / "world"; }

    std::filesystem::path debugDir() const { return dataDir() / "debug"; }

    bool importWorld(const std::vector<std::byte>& worldData);

    RunStatus run(const std::string& worldUUID = {}, bool simulate = false, bool online = false, bool power = false, bool run = false);
    void exit();
};

#endif
