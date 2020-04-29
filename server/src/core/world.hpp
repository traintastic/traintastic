/**
 * server/src/core/world.hpp
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

#ifndef TRAINTASTIC_SERVER_CORE_WORLD_HPP
#define TRAINTASTIC_SERVER_CORE_WORLD_HPP

#include "object.hpp"
#include "property.hpp"
#include "objectproperty.hpp"
#include "stdfilesystem.hpp"
#include <unordered_map>
#include <boost/uuid/uuid.hpp>
#include <nlohmann/json.hpp>
#include <enum/worldevent.hpp>
#include <enum/worldscale.hpp>
#include <set/worldstate.hpp>
#include "../clock/clock.hpp"
#include "../hardware/commandstation/commandstationlist.hpp"
#include "../hardware/decoder/decoderlist.hpp"
#include "../hardware/input/inputlist.hpp"
#include "../hardware/controller/controllerlist.hpp"
#include "../lua/scriptlist.hpp"
//class CommandStationList;
//class DecoderList;

class WorldLoader;

class World : public Object
{
  friend class IdObject;
  friend class Traintastic;
  friend class WorldLoader;
  friend class WorldSaver;

  protected:
    static void init(const std::shared_ptr<World>& world);

    std::filesystem::path m_filename;
    boost::uuids::uuid m_uuid;
    std::unordered_map<std::string, std::weak_ptr<Object>> m_objects;

    void event(WorldEvent event);
    void load();

    nlohmann::json saveObject(const ObjectPtr& object);

  public:
    CLASS_ID("world")

    static std::shared_ptr<World> create();
    static std::shared_ptr<World> load(const std::filesystem::path& filename);

    Property<std::string> name;
    Property<WorldScale> scale;

    ObjectProperty<CommandStationList> commandStations;
    ObjectProperty<DecoderList> decoders;
    ObjectProperty<InputList> inputs;
    ObjectProperty<ControllerList> controllers;
    ObjectProperty<Clock> clock;
    ObjectProperty<Lua::ScriptList> luaScripts;

    Property<WorldState> state;
    Method<void()> emergencyStop;
    Method<void()> trackPowerOff;
    Method<void()> trackPowerOn;
    Property<bool> edit;

    Method<void()> save;

    World(); // Don't use directly, use: create()
    World(const std::filesystem::path& filename); // Don't use directly, use: load()

    const boost::uuids::uuid& uuid() const { return m_uuid; }

    std::string getUniqueId(const std::string& prefix) const;
    //ObjectPtr createObject(const std::string& classId, std::string_view _id = "");
    bool isObject(const std::string&_id) const;
    ObjectPtr getObject(const std::string& _id) const;
};

#endif
