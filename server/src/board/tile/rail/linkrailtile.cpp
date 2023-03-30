/**
 * server/src/board/tile/rail/linkrailtile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#include "linkrailtile.hpp"
#include "../../list/linkrailtilelist.hpp"
#include "../../../core/attributes.hpp"
#include "../../../core/objectlisttablemodel.hpp"
#include "../../../core/objectproperty.tpp"
#include "../../../utils/displayname.hpp"
#include "../../../world/world.hpp"

LinkRailTile::LinkRailTile(World& world, std::string_view _id)
  : RailTile(world, _id, TileId::RailLink)
  , m_node{*this, 1}
  , name{this, "name", id, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly}
  , link{this, "link", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::NoScript, nullptr,
      [this](const std::shared_ptr<LinkRailTile>& newValue)
      {
        if(newValue.get() == this)
          return false;

        if(link)
        {
          assert(link->link.value().get() == this);
          link->link.setValueInternal(nullptr);
        }

        if(newValue)
        {
          if(newValue->link)
          {
            assert(newValue->link->link.value() == newValue);
            newValue->link->link.setValueInternal(nullptr);
          }
          newValue->link.setValueInternal(shared_ptr<LinkRailTile>());
        }

        return true;
      }}
{
  const auto worldState = m_world.state.value();
  const bool editable = contains(worldState, WorldState::Edit);
  const bool running = contains(worldState, WorldState::Run);

  Attributes::addEnabled(name, editable);
  Attributes::addDisplayName(name, DisplayName::Object::name);
  m_interfaceItems.add(name);

  Attributes::addEnabled(link, editable && !running);
  Attributes::addObjectList(link, m_world.linkRailTiles);
  m_interfaceItems.add(link);
}

void LinkRailTile::getConnectors(std::vector<Connector>& connectors) const
{
  connectors.emplace_back(location(), rotate, Connector::Type::Rail);
}

void LinkRailTile::addToWorld()
{
  RailTile::addToWorld();

  m_world.linkRailTiles->addObject(shared_ptr<LinkRailTile>());
}

void LinkRailTile::loaded()
{
  RailTile::loaded();

  // check if links refer to eachother, else disconnect:
  if(link && link->link.value().get() != this)
  {
    //! \todo warning?
    link.setValueInternal(nullptr);
  }
}

void LinkRailTile::destroying()
{
  m_world.linkRailTiles->removeObject(shared_ptr<LinkRailTile>());

  RailTile::destroying();
}

void LinkRailTile::worldEvent(WorldState worldState, WorldEvent worldEvent)
{
  RailTile::worldEvent(worldState, worldEvent);

  const bool editable = contains(worldState, WorldState::Edit);
  const bool running = contains(worldState, WorldState::Run);

  Attributes::setEnabled(name, editable);
  Attributes::setEnabled(link, editable && !running);
}
