/**
 * server/src/world/world.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_WORLD_WORLD_HPP
#define TRAINTASTIC_SERVER_WORLD_WORLD_HPP

#include "../core/object.hpp"
#include "../core/property.hpp"
#include "../core/objectproperty.hpp"
#include <traintastic/utils/stdfilesystem.hpp>
#include <unordered_map>
#include <boost/uuid/uuid.hpp>
#include <traintastic/enum/worldevent.hpp>
#include "../enum/worldscale.hpp"
#include <traintastic/set/worldstate.hpp>
#include "../clock/clock.hpp"
#include "../board/boardlist.hpp"
#include "../hardware/commandstation/commandstationlist.hpp"
#include "../hardware/decoder/decoderlist.hpp"
#include "../hardware/input/list/inputlist.hpp"
#include "../hardware/output/list/outputlist.hpp"
#include "../hardware/controller/controllerlist.hpp"
#include "../hardware/protocol/loconet/loconetlist.hpp"
#include "../hardware/protocol/xpressnet/xpressnetlist.hpp"
#include "../train/trainlist.hpp"
#include "../vehicle/rail/railvehiclelist.hpp"
#ifndef DISABLE_LUA_SCRIPTING
  #include "../lua/scriptlist.hpp"
#endif

class WorldLoader;

class World : public Object
{
  friend class IdObject;
  friend class Traintastic;
  friend class WorldLoader;
  friend class WorldSaver;

  private:
    struct Private {};

    void updateScaleRatio();

  protected:
    static void init(const std::shared_ptr<World>& world);

    std::unordered_map<std::string, std::weak_ptr<Object>> m_objects;

    void loaded() final;
    void worldEvent(WorldState worldState, WorldEvent worldEvent) final;
    void event(WorldEvent value);

  public:
    CLASS_ID("world")

    static constexpr std::string_view id = classId;
    static constexpr std::string_view dotCTW = ".ctw";
    static constexpr std::string_view filename = "traintastic.json";
    static constexpr std::string_view filenameState = "traintastic.state.json";

    static std::shared_ptr<World> create();

    Property<std::string> uuid;
    Property<std::string> name;
    Property<WorldScale> scale;
    Property<double> scaleRatio;

    ObjectProperty<CommandStationList> commandStations;
    ObjectProperty<DecoderList> decoders;
    ObjectProperty<InputList> inputs;
    ObjectProperty<OutputList> outputs;
    ObjectProperty<ControllerList> controllers;
    ObjectProperty<LocoNetList> loconets;
    ObjectProperty<XpressNetList> xpressnets;
    ObjectProperty<BoardList> boards;
    ObjectProperty<Clock> clock;
    ObjectProperty<TrainList> trains;
    ObjectProperty<RailVehicleList> railVehicles;
#ifndef DISABLE_LUA_SCRIPTING
    ObjectProperty<Lua::ScriptList> luaScripts;
#endif

    Property<WorldState> state;
    Property<bool> edit;
    Method<void()> offline;
    Method<void()> online;
    Method<void()> powerOff;
    Method<void()> powerOn;
    Method<void()> run;
    Method<void()> stop;
    Property<bool> mute;
    Property<bool> noSmoke;

    Method<void()> save;

    World(Private);

    std::string getObjectId() const final { return std::string(classId); }

    //const boost::uuids::uuid& uuid() const { return m_uuid; }

    std::string getUniqueId(std::string_view prefix) const;
    bool isObject(const std::string&_id) const;
    ObjectPtr getObject(const std::string& _id) const;
    ObjectPtr getObjectByPath(std::string_view path) const;
};

#endif
