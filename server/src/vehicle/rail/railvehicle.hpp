/**
 * server/src/vehicle/rail/railvehicle.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021,2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_VEHICLE_RAIL_RAILVEHICLE_HPP
#define TRAINTASTIC_SERVER_VEHICLE_RAIL_RAILVEHICLE_HPP

#include "../vehicle.hpp"
#include "../../core/objectproperty.hpp"
#include "../../core/lengthproperty.hpp"
#include "../../core/speedproperty.hpp"
#include "../../core/weightproperty.hpp"
#include "../../core/trainproperty.hpp"

class Decoder;

class RailVehicle : public Vehicle
{
  protected:
    RailVehicle(World& world, std::string_view _id);

    void addToWorld() override;
    void destroying() override;
    void loaded() override;
    void worldEvent(WorldState state, WorldEvent event) override;

    virtual double calcTotalWeight(WeightUnit unit) const;
    void updateTotalWeight();

  public:
    ObjectProperty<Decoder> decoder;
    LengthProperty lob;
    SpeedProperty speedMax;
    WeightProperty weight;
    WeightProperty totalWeight;

    TrainProperty activeTrain;
};

#endif
