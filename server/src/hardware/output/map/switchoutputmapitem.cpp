/**
 * server/src/hardware/output/map/switchoutputmapitem.cpp
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

#include "switchoutputmapitem.hpp"
#include "outputmapitembase.tpp"

static constexpr std::array<bool, 2> keyAliasKeys{{false, true}};
static const std::array<std::string, 2> keyAliasValues{{"$output_map_item.switch.key:off$", "$output_map_item.switch.key:on$"}};

SwitchOutputMapItem::SwitchOutputMapItem(Object& map, bool value) :
  OutputMapItemBase(map, value)
{
  Attributes::addAliases(key, tcb::span<const bool>(keyAliasKeys), tcb::span<const std::string>(keyAliasValues));
}
