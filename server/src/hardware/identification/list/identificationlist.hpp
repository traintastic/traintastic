/**
 * server/src/hardware/identification/list/inpulist.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_IDENTIFICATION_LIST_IDENTIFICATIONLIST_HPP
#define TRAINTASTIC_SERVER_HARDWARE_IDENTIFICATION_LIST_IDENTIFICATIONLIST_HPP

#include "../../../core/objectlist.hpp"
#include "identificationlistcolumn.hpp"
#include "../../../core/method.hpp"

class Identification;

class IdentificationList : public ObjectList<Identification>
{
  CLASS_ID("list.identification")

  protected:
    void worldEvent(WorldState state, WorldEvent event) final;
    bool isListedProperty(std::string_view name) final;

  public:
    const IdentificationListColumn columns;

    Method<std::shared_ptr<Identification>()> create;
    Method<void(const std::shared_ptr<Identification>&)> delete_;

    IdentificationList(Object& _parent, std::string_view parentPropertyName, IdentificationListColumn _columns);

    TableModelPtr getModel() final;
};

#endif
