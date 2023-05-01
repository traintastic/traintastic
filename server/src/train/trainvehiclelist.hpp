/**
 * server/src/train/trainvehiclelist.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_TRAIN_TRAINVEHICLELIST_HPP
#define TRAINTASTIC_SERVER_TRAIN_TRAINVEHICLELIST_HPP

#include "../core/subobject.hpp"
#include "../core/objectvectorproperty.hpp"
#include "../core/table.hpp"
#include "../core/method.hpp"

class Train;
class TrainVehicleListItem;
class TrainVehicleListTableModel;
class RailVehicle;

class TrainVehicleList : public SubObject, public Table
{
  CLASS_ID("list.train_vehicle")

  private:
    friend class TrainVehicleListItem;
    friend class TrainVehicleListTableModel;
    inline Train& train();
    uint32_t getItemId() const;

  protected:
    std::unordered_map<Object*, boost::signals2::connection> m_propertyChanged;
    std::vector<TrainVehicleListTableModel *> m_models;

    void load(WorldLoader& loader, const nlohmann::json& data) final;

    void propertyChanged(BaseProperty& property);

    void rowCountChanged();

    void rowsChanged(uint32_t first, uint32_t last);

    void addObject(std::shared_ptr<RailVehicle> vehicle);

    void removeObject(const std::shared_ptr<TrainVehicleListItem>& item);

  public:
    using const_iterator = ObjectVectorProperty<TrainVehicleListItem>::const_iterator;

    ObjectVectorProperty<TrainVehicleListItem> items;
    Method<void(const std::shared_ptr<RailVehicle>&)> add;
    Method<void(const std::shared_ptr<TrainVehicleListItem>&)> remove;
    Method<void(uint32_t, uint32_t)> move;
    Method<void()> reverse;

    TrainVehicleList(Train& train_, std::string_view parentPropertyName);

    inline const_iterator begin() const { return items.begin(); }
    inline const_iterator end() const { return items.end(); }
    inline bool empty() const { return items.empty(); }

    TableModelPtr getModel() final;

    std::shared_ptr<TrainVehicleListItem> getItemFromVehicle(const std::shared_ptr<RailVehicle>& vehicle) const;
};

#endif
