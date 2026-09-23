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

#include "trainroute.hpp"
#include "trainrouteentry.hpp"
#include "trainroutelist.hpp"
#include "trainroutelisttablemodel.hpp"
#include "../board/tile/rail/blockrailtile.hpp"
#include "../board/pathfinder/trainpathfinder.hpp"
#include "../core/attributes.hpp"
#include "../core/method.tpp"
#include "../core/objectproperty.tpp"
#include "../core/objectvectorproperty.tpp"
#include "../hardware/trackdriver/blocktrackdriver.hpp"
#include "../train/train.hpp"
#include "../train/trainblockstatus.hpp"
#include "../utils/displayname.hpp"
#include "../world/world.hpp"
#include "../zone/blockzonelist.hpp"

CREATE_IMPL(TrainRoute)

TrainRoute::TrainRoute(World& world, std::string_view _id)
  : IdObject(world, _id)
  , name{this, "name", id, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly}
  , enabled{this, "enabled", false, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadWrite}
  , temporary{this, "temporary", false, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::ScriptReadOnly}
  , valid{this, "valid", false, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly}
  , entries{*this, "entries", {}, PropertyFlags::ReadOnly | PropertyFlags::Store | PropertyFlags::SubObject | PropertyFlags::ScriptReadOnly}
  , entriesResolved{*this, "entries_resolved", {}, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly}
  , addBlock{*this, "add_block",
      [this](const std::shared_ptr<BlockRailTile>& block)
      {
        entries.appendInternal(std::make_shared<TrainRouteEntry>(*this, *block));
        resolve();
      }}
  , remove{*this, "remove",
      [this](const std::shared_ptr<TrainRouteEntry>& entry)
      {
        if(entry && entries.removeInternal(entry))
        {
          entry->destroy();
          resolve();
        }
      }}
{
  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, false);
  m_interfaceItems.add(name);

  m_interfaceItems.add(enabled);

  Attributes::addObjectEditor(temporary, false);
  m_interfaceItems.add(temporary);

  Attributes::addObjectEditor(valid, false);
  m_interfaceItems.add(valid);

  Attributes::addObjectEditor(entries, false);
  m_interfaceItems.add(entries);

  Attributes::addObjectEditor(entriesResolved, false);
  m_interfaceItems.add(entriesResolved);

  Attributes::addEnabled(addBlock, false);
  Attributes::addObjectEditor(addBlock, false);
  Attributes::addObjectList(addBlock, world.blockRailTiles);
  m_interfaceItems.add(addBlock);

  Attributes::addEnabled(remove, false);
  Attributes::addObjectEditor(remove, false);
  m_interfaceItems.add(remove);

  updateEnabled();
}

void TrainRoute::loaded()
{
  IdObject::loaded();
  resolve();
}

void TrainRoute::addToWorld()
{
  IdObject::addToWorld();
  m_world.trainRoutes->addObject(shared_ptr<TrainRoute>());
}

void TrainRoute::destroying()
{
  auto self = shared_ptr<TrainRoute>();
  m_world.trainRoutes->removeObject(self);
  IdObject::destroying();
}

void TrainRoute::worldEvent(WorldState state, WorldEvent event)
{
  IdObject::worldEvent(state, event);

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

void TrainRoute::resolve()
{
  if(entries.size() < 2) // we need at least two entries for a route
  {
    entriesResolved.setValuesInternal(entries);
    valid.setValueInternal(false);
    return;
  }

  std::vector<std::shared_ptr<TrainRouteEntry>> route;
  route.emplace_back(entries.front());

  std::optional<BlockSide> toSide;
  bool invalid = false;
  for(auto it = ++entries.begin(); it != entries.end(); ++it)
  {
    const auto& from = route.back();
    const auto fromSide = toSide;// ? ~*toSide : toSide;
    const auto& to = *it;
    toSide = std::nullopt;

    if(const auto blocks = m_world.trainPathFinder->find(*from->block, fromSide, *to->block, toSide); blocks.size() >= 2)
    {
      for(size_t i = 1; i < blocks.size() - 1; ++i)
      {
        assert(blocks[i]);
        // FIXME: recycle existing objects
        route.emplace_back(std::make_shared<TrainRouteEntry>(*this, *blocks[i]))->source.setValueInternal(TrainRouteEntrySource::Resolver);
      }
    }
    else // insert dummy item indicating "no path"
    {
      // FIXME: recycle existing objects
      route.emplace_back(std::make_shared<TrainRouteEntry>(*this))->source.setValueInternal(TrainRouteEntrySource::Resolver);
      invalid = true;
    }

    route.emplace_back(to);
  }

  entriesResolved.setValuesInternal(route);
  valid.setValueInternal(!invalid);
}

void TrainRoute::updateEnabled()
{
  const bool editable = contains(m_world.state, WorldState::Edit);

  Attributes::setEnabled(name, editable);
  Attributes::setEnabled(addBlock, editable);
  Attributes::setEnabled(remove, editable);
}
