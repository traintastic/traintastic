/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#include "trainrouteentry.hpp"
#include "trainroute.hpp"
#include "../board/tile/rail/blockrailtile.hpp"
#include "../core/attributes.hpp"
#include "../core/objectproperty.tpp"
#include "../utils/tohex.hpp"
#include "../utils/unit.hpp"
#include "../world/world.hpp"

namespace {

constexpr std::pair<uint16_t, uint16_t> waitTimeRange{0, 3600}; // 0 sec .. 1 hour

}

TrainRouteEntry::TrainRouteEntry(TrainRoute& route)
  : Object()
  , block{this, "block", nullptr, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::ScriptReadOnly}
  , waitTimeMin{this, "wait_time_min", 0, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly, nullptr,
      [this](uint16_t& value)
      {
        value = std::clamp(value, waitTimeRange.first, waitTimeRange.second);
        if(value > waitTimeMax)
        {
          waitTimeMax.setValueInternal(value);
        }
        return true;
      }}
  , waitTimeMax{this, "wait_time_max", 0, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly, nullptr,
      [this](uint16_t& value)
      {
        value = std::clamp(value, waitTimeRange.first, waitTimeRange.second);
        if(value < waitTimeMin)
        {
          waitTimeMin.setValueInternal(value);
        }
        return true;
      }}
  , source{this, "source", TrainRouteEntrySource::Explicit, PropertyFlags::ReadOnly | PropertyFlags::NoStore}
  , m_route{route}
{
  m_interfaceItems.add(block);

  Attributes::addEnabled(waitTimeMin, false);
  Attributes::addMinMax(waitTimeMin, waitTimeRange);
  Attributes::addUnit(waitTimeMin, Unit::seconds);
  m_interfaceItems.add(waitTimeMin);

  Attributes::addEnabled(waitTimeMax, false);
  Attributes::addMinMax(waitTimeMax, waitTimeRange);
  Attributes::addUnit(waitTimeMax, Unit::seconds);
  m_interfaceItems.add(waitTimeMax);

  Attributes::addObjectEditor(source, false);
  Attributes::addValues(source, trainRouteEntrySourceValues);
  m_interfaceItems.add(source);

  updateEnabled();
}

TrainRouteEntry::TrainRouteEntry(TrainRoute& route, BlockRailTile& block_)
  : TrainRouteEntry(route)
{
  block.setValueInternal(block_.shared_ptr<BlockRailTile>());
}

std::string TrainRouteEntry::getObjectId() const
{
  std::string id{"train_route_entry_"};
  id.append(toHex(reinterpret_cast<uintptr_t>(this)));
  return id;
}

void TrainRouteEntry::worldEvent(WorldState state, WorldEvent event)
{
  Object::worldEvent(state, event);

  switch(event)
  {
    case WorldEvent::EditEnabled:
    case WorldEvent::EditDisabled:
      updateEnabled();
      break;

    default:
      break;
  }
}

void TrainRouteEntry::updateEnabled()
{
  const bool editable = contains(m_route.world().state, WorldState::Edit);

  Attributes::setEnabled({waitTimeMin, waitTimeMax}, editable);
}
