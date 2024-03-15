/**
 * server/src/world/world.hpp
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

#ifndef TRAINTASTIC_SERVER_WORLD_WORLD_HPP
#define TRAINTASTIC_SERVER_WORLD_WORLD_HPP

#include "../core/object.hpp"
#include "../core/property.hpp"
#include "../core/objectproperty.hpp"
#include "../core/objectvectorproperty.hpp"
#include "../core/method.hpp"
#include "../core/event.hpp"
#include <unordered_map>
#include <boost/uuid/uuid.hpp>
#include <traintastic/enum/worldevent.hpp>
#include "../enum/worldscale.hpp"
#include "../status/status.hpp"
#include <traintastic/set/worldstate.hpp>

class WorldLoader;
class LNCVProgrammer;
class DecoderController;
class InputController;
class OutputController;
class IdentificationController;
class LNCVProgrammingController;
class InterfaceList;
class DecoderList;
class InputList;
class OutputList;
class IdentificationList;
class BoardList;
class LinkRailTileList;
class NXManager;
class Clock;
class TrainList;
class RailVehicleList;

template <typename T>
class ControllerList;

namespace Lua {
  class ScriptList;
}

class World : public Object
{
  friend class IdObject;
  friend class StateObject;
  friend class Traintastic;
  friend class WorldLoader;
  friend class WorldSaver;

  private:
    struct Private {};

    void updateEnabled();
    void updateScaleRatio();

  protected:
    static void init(World& world);

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
    Property<bool> onlineWhenLoaded;
    Property<bool> powerOnWhenLoaded;
    Property<bool> runWhenLoaded;

    ObjectProperty<ControllerList<DecoderController>> decoderControllers;
    ObjectProperty<ControllerList<InputController>> inputControllers;
    ObjectProperty<ControllerList<OutputController>> outputControllers;
    ObjectProperty<ControllerList<IdentificationController>> identificationControllers;
    ObjectProperty<ControllerList<LNCVProgrammingController>> lncvProgrammingControllers;

    ObjectProperty<InterfaceList> interfaces;
    ObjectProperty<DecoderList> decoders;
    ObjectProperty<InputList> inputs;
    ObjectProperty<OutputList> outputs;
    ObjectProperty<IdentificationList> identifications;
    ObjectProperty<BoardList> boards;
    ObjectProperty<Clock> clock;
    ObjectProperty<TrainList> trains;
    ObjectProperty<RailVehicleList> railVehicles;
    ObjectProperty<Lua::ScriptList> luaScripts;

    ObjectProperty<LinkRailTileList> linkRailTiles;
    ObjectProperty<NXManager> nxManager;

    ObjectVectorProperty<Status> statuses;
    Property<uint32_t> hardwareThrottles; //<! number of connected hardware throttles

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
    Property<bool> simulation;

    Method<void()> save;

    Method<ObjectPtr(const std::string&)> getObject_;

    Method<std::shared_ptr<LNCVProgrammer>(const ObjectPtr&)> getLNCVProgrammer;

    Event<WorldState, WorldEvent> onEvent;

    World(Private);
    ~World() override;

    std::string getObjectId() const final { return std::string(classId); }

    std::string getUniqueId(std::string_view prefix) const;
    bool isObject(const std::string&_id) const;
    ObjectPtr getObjectById(const std::string& _id) const;
    ObjectPtr getObjectByPath(std::string_view path) const;

    void export_(std::vector<std::byte>& data);
};

#endif
