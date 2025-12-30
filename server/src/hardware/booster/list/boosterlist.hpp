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

#ifndef TRAINTASTIC_SERVER_HARDWARE_BOOSTER_LIST_BOOSTERLIST_HPP
#define TRAINTASTIC_SERVER_HARDWARE_BOOSTER_LIST_BOOSTERLIST_HPP

#include "../../../core/objectlist.hpp"
#include "../../../core/method.hpp"
#include "../booster.hpp"

class BoosterList final : public ObjectList<Booster>
{
  CLASS_ID("list.booster")

protected:
  void worldEvent(WorldState state, WorldEvent event) final;
  bool isListedProperty(std::string_view name) final;

public:
  Method<std::shared_ptr<Booster>()> create;
  Method<void(const std::shared_ptr<Booster>&)> delete_;

  BoosterList(Object& _parent, std::string_view parentPropertyName);
  ~BoosterList() final = default;

  TableModelPtr getModel() final;
};

#endif
