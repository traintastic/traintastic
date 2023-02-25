/**
 * server/src/vehicle/rail/train.hpp
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

#ifndef TRAINTASTIC_SERVER_TRAIN_TRAINLIST_HPP
#define TRAINTASTIC_SERVER_TRAIN_TRAINLIST_HPP

#include "../core/objectlist.hpp"
#include "../core/method.hpp"
#include "train.hpp"

class TrainList : public ObjectList<Train>
{
  protected:
    void worldEvent(WorldState state, WorldEvent event) final;
    bool isListedProperty(std::string_view name) final;

  public:
    CLASS_ID("list.train")

    Method<std::shared_ptr<Train>()> create;
    Method<void(const std::shared_ptr<Train>&)> delete_;

    TrainList(Object& _parent, std::string_view parentPropertyName);

    TableModelPtr getModel() final;
};

#endif
