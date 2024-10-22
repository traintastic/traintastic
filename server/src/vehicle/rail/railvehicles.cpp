/**
 * server/src/vehicle/rail/railvehicles.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2023-2024 Reinder Feenstra
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

#include "railvehicles.hpp"
#include "../../utils/ifclassidcreate.hpp"
#include "../../utils/makearray.hpp"

#include "locomotive.hpp"
#include "multipleunit.hpp"
#include "freightwagon.hpp"
#include "tankwagon.hpp"

tcb::span<const std::string_view> RailVehicles::classList()
{
  static constexpr auto classes = makeArray(
    Locomotive::classId,
    MultipleUnit::classId,
    FreightWagon::classId,
    TankWagon::classId
  );
  return classes;
}

std::shared_ptr<RailVehicle> RailVehicles::create(World& world, std::string_view classId, std::string_view id)
{
  if(classId == Locomotive::classId)
    return Locomotive::create(world, id);
  if(classId == MultipleUnit::classId)
    return MultipleUnit::create(world, id);
  if(classId == FreightWagon::classId)
    return FreightWagon::create(world, id);
  if(classId == TankWagon::classId)
    return TankWagon::create(world, id);
  return std::shared_ptr<RailVehicle>();
}
