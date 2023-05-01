/**
 * server/src/train/trainvehiclelistitem.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Filippo Gentile
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

#ifndef TRAINTASTIC_SERVER_TRAIN_TRAINVEHICLELISTITEM_HPP
#define TRAINTASTIC_SERVER_TRAIN_TRAINVEHICLELISTITEM_HPP

#include "../core/object.hpp"
#include "../core/property.hpp"
#include "../core/objectproperty.hpp"

class TrainVehicleList;
class RailVehicle;

class TrainVehicleListItem : public Object
{
  public:
    CLASS_ID("train_vehicle.item")

  private:
    TrainVehicleList &m_parent;
    const uint32_t m_itemId;
    boost::signals2::connection m_vehiclePropertyChanged;
    boost::signals2::connection m_vehicleDestroying;

  protected:
    void save(WorldSaver& saver, nlohmann::json& data, nlohmann::json& state) const final;
    void loaded() final;

    void connectVehicle(RailVehicle &object);
    void disconnectVehicle(RailVehicle &);

  public:
    ObjectProperty<RailVehicle> vehicle;
    Property<bool> invertDirection;

    TrainVehicleListItem(TrainVehicleList& parent, uint32_t itemId);

    std::string getObjectId() const final;
    uint32_t itemId() const { return m_itemId; }
};

#endif // TRAINVEHICLELISTITEM_H
