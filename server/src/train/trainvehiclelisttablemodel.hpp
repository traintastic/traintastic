/**
 * server/src/train/trainvehiclelisttablemodel.hpp
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

#ifndef TRAINTASTIC_SERVER_TRAIN_TRAINVEHICLELISTTABLEMODEL_HPP
#define TRAINTASTIC_SERVER_TRAIN_TRAINVEHICLELISTTABLEMODEL_HPP

#include "../core/tablemodel.hpp"

class TrainVehicleList;

class TrainVehicleListTableModel : public TableModel
{
    friend class TrainVehicleList;
  private:
    std::shared_ptr<TrainVehicleList> m_list;

  protected:
    void propertyChanged(BaseProperty& property, uint32_t row);

  public:
    CLASS_ID("train_vehicle_list_table_model")

    static bool isListedProperty(std::string_view name);

    TrainVehicleListTableModel(TrainVehicleList &list);
    ~TrainVehicleListTableModel() override;

    std::string getText(uint32_t column, uint32_t row) const final;
};

#endif // TRAINVEHICLELISTTABLEMODEL_HPP
