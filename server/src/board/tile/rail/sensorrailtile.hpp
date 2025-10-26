/**
 * server/src/board/tile/rail/sensorrailtile.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020-2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_BOARD_TILE_RAIL_SENSORRAILTILE_HPP
#define TRAINTASTIC_SERVER_BOARD_TILE_RAIL_SENSORRAILTILE_HPP

#include "straightrailtile.hpp"
#include "../../../core/method.hpp"
#include "../../../hardware/input/input.hpp"
#include "../../../hardware/input/inputconsumer.hpp"
#include "../../../enum/sensortype.hpp"
#include "../../../enum/sensorstate.hpp"

class SensorRailTile : public StraightRailTile, public InputConsumer
{
  CLASS_ID("board_tile.rail.sensor")
  DEFAULT_ID("sensor")
  CREATE(SensorRailTile)

  private:
    void updateSimulateTriggerEnabled();

  protected:
    void load(WorldLoader& loader, const nlohmann::json& data) override;
    void loaded() override;
    void destroying() override;
    void worldEvent(WorldState worldState, WorldEvent worldEvent) override;
    void inputValueChanged(bool value, const std::shared_ptr<Input>& input) override;

  public:
    Property<std::string> name;
    Property<SensorType> type;
    Property<bool> invert;
    Property<SensorState> state;
    Method<void()> simulateTrigger;

    SensorRailTile(World& world, std::string_view _id);
};

#endif
