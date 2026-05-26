/**
 * server/src/zone/zone.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_ZONE_ZONE_HPP
#define TRAINTASTIC_SERVER_ZONE_ZONE_HPP

#include "../core/idobject.hpp"
#include "../core/event.hpp"
#include "../core/objectproperty.hpp"
#include "../core/objectvectorproperty.hpp"
#include "../core/speedlimitproperty.hpp"
#include "../train/trainzonestatus.hpp"

class ZoneBlockList;
class Train;

class Zone : public IdObject
{
  friend class TrainTracking;

  CLASS_ID("zone")
  CREATE_DEF(Zone)
  DEFAULT_ID("zone")

protected:
  void worldEvent(WorldState worldState, WorldEvent worldEvent) override;
  void addToWorld() override;
  void destroying() override;

  void fireTrainAssigned(const std::shared_ptr<Train>& train);
  void fireTrainEntering(const std::shared_ptr<Train>& train);
  void fireTrainEntered(const std::shared_ptr<Train>& train);
  void fireTrainLeaving(const std::shared_ptr<Train>& train);
  void fireTrainLeft(const std::shared_ptr<Train>& train);
  void fireTrainRemoved(const std::shared_ptr<Train>& train);

public:
  Zone(World& world, std::string_view id_);

  std::shared_ptr<TrainZoneStatus> getTrainZoneStatus(const std::shared_ptr<Train>& train);

  Property<std::string> name;
  Property<bool> mute;
  Property<bool> noSmoke;
  SpeedLimitProperty speedLimit;
  ObjectProperty<ZoneBlockList> blocks;
  ObjectVectorProperty<TrainZoneStatus> trains;
  Event<const std::shared_ptr<Train>&, const std::shared_ptr<Zone>&> onTrainAssigned;
  Event<const std::shared_ptr<Train>&, const std::shared_ptr<Zone>&> onTrainEntering;
  Event<const std::shared_ptr<Train>&, const std::shared_ptr<Zone>&> onTrainEntered;
  Event<const std::shared_ptr<Train>&, const std::shared_ptr<Zone>&> onTrainLeaving;
  Event<const std::shared_ptr<Train>&, const std::shared_ptr<Zone>&> onTrainLeft;
  Event<const std::shared_ptr<Train>&, const std::shared_ptr<Zone>&> onTrainRemoved;
};

#endif
