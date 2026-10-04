/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_BOOSTER_LIST_BOOSTERLISTTABLEMODEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_BOOSTER_LIST_BOOSTERLISTTABLEMODEL_HPP

#include "../../../core/objectlisttablemodel.hpp"
#include "boosterlist.hpp"

class BoosterList;

class BoosterListTableModel : public ObjectListTableModel<Booster>
{
  friend class BoosterList;

  protected:
    void propertyChanged(BaseProperty& property, uint32_t row) final;

  public:
    CLASS_ID("table_model.list.booster")

    static bool isListedProperty(std::string_view name);

    BoosterListTableModel(BoosterList& boosterList);

    std::string getText(uint32_t column, uint32_t row) const final;
};

#endif
