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

#include "turnoutlinkablerailtile.hpp"
#include "../../../list/turnoutlinkablerailtilelist.hpp"
#include "../../../list/turnoutlinkablerailtilelisttablemodel.hpp"
#include "../../../../core/method.tpp"
#include "../../../../core/objectproperty.tpp"
#include "../../../../utils/category.hpp"

TurnoutLinkableRailTile::TurnoutLinkableRailTile(World& world, std::string_view id_, TileId tileId_)
  : TurnoutRailTile(world, id_, tileId_, 3)
  , linked{this, "linked", false, PropertyFlags::ReadWrite | PropertyFlags::Store,
      [this](bool /*value*/)
      {
        linkedChanged();
      }}
  , linkTurnout{this, "link_turnout", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store,
      [this](const std::shared_ptr<TurnoutLinkableRailTile>& value)
      {
        if(value)
        {
          syncPosition();
        }
      },
      [this](const std::shared_ptr<TurnoutLinkableRailTile>& value)
      {
        if(linkTurnout)
        {
          linkTurnout->unlinkTurnout(*this);
        }

        if(value)
        {
          value->m_linkedTurnouts.emplace_back(shared_ptr<TurnoutLinkableRailTile>());
          value->updateEnabled();
        }

        return true;
      }}
  , linkInvert{this, "link_invert", false, PropertyFlags::ReadWrite | PropertyFlags::Store,
      [this](bool /*value*/)
      {
        if(linkTurnout)
        {
          syncPosition();
        }
      }}
{
  const bool editable = contains(m_world.state.value(), WorldState::Edit);
  const bool run = contains(m_world.state.value(), WorldState::Run);

  Attributes::addCategory(linked, Category::options);
  Attributes::addDisplayName(linked, "board_tile.rail.turnout:linked");
  Attributes::addEnabled(linked, editable && !run);
  m_interfaceItems.add(linked);

  Attributes::addCategory(linkTurnout, Category::options);
  Attributes::addDisplayName(linkTurnout, "board_tile.rail.turnout:link_turnout");
  Attributes::addEnabled(linkTurnout, editable && !run);
  Attributes::addObjectList(linkTurnout, m_world.turnoutLinkableRailTiles);
  Attributes::addVisible(linkTurnout, false);
  m_interfaceItems.add(linkTurnout);

  Attributes::addCategory(linkInvert, Category::options);
  Attributes::addDisplayName(linkInvert, "board_tile.rail.turnout:link_invert");
  Attributes::addEnabled(linkInvert, editable && !run);
  Attributes::addVisible(linkInvert, false);
  m_interfaceItems.add(linkInvert);
}

void TurnoutLinkableRailTile::addToWorld()
{
  TurnoutRailTile::addToWorld();

  if(!linked)
  {
    m_world.turnoutLinkableRailTiles->addObject(shared_ptr<TurnoutLinkableRailTile>());
  }
}

void TurnoutLinkableRailTile::loaded()
{
  TurnoutRailTile::loaded();
  linkedChanged();
  if(linkTurnout)
  {
    assert(linked);
    linkTurnout->m_linkedTurnouts.emplace_back(shared_ptr<TurnoutLinkableRailTile>());
    linkTurnout->updateEnabled();
  }
}

void TurnoutLinkableRailTile::destroying()
{
  if(linkTurnout)
  {
    linkTurnout->unlinkTurnout(*this);
  }

  m_world.turnoutLinkableRailTiles->removeObject(shared_ptr<TurnoutLinkableRailTile>());

  TurnoutRailTile::destroying();
}

void TurnoutLinkableRailTile::worldEvent(WorldState state, WorldEvent event)
{
  TurnoutRailTile::worldEvent(state, event);
  switch(event)
  {
    case WorldEvent::EditDisabled:
    case WorldEvent::EditEnabled:
    case WorldEvent::Run:
    case WorldEvent::Stop:
    {
      updateEnabled();
      break;
    }
    default:
      break;
  }
}

bool TurnoutLinkableRailTile::doSetPosition(TurnoutPosition value, bool skipAction)
{
  if(!linked)
  {
    return TurnoutRailTile::doSetPosition(value, skipAction);
  }
  else if(linkTurnout)
  {
    return linkTurnout->setPosition(convertPosition(*this, *linkTurnout, value, linkInvert));
  }
  return false;
}

void TurnoutLinkableRailTile::newPosition(TurnoutPosition value)
{
  TurnoutRailTile::newPosition(value);
  if(!m_linkedTurnouts.empty())
  {
    assert(!linked);
    assert(!linkTurnout);
    for(const auto& turnout : m_linkedTurnouts)
    {
      turnout->syncPosition();
    }
  }
}

void TurnoutLinkableRailTile::linkedChanged()
{
  auto self = shared_ptr<TurnoutLinkableRailTile>();
  if(linked)
  {
    outputMap->interface = nullptr;
    feedbackMap->interface = nullptr;
    m_world.turnoutLinkableRailTiles->removeObject(self);
  }
  else
  {
    linkTurnout = nullptr;

    if(!m_world.turnoutLinkableRailTiles->containsObject(self))
    {
      m_world.turnoutLinkableRailTiles->addObject(std::move(self));
    }
  }
  updateVisible();
}

void TurnoutLinkableRailTile::updateEnabled()
{
  const auto state = m_world.state.value();
  const bool editable = contains(state, WorldState::Edit);
  const bool run = contains(state, WorldState::Run);
  Attributes::setEnabled({linked, linkTurnout, linkInvert}, editable && !run && m_linkedTurnouts.empty());
}

void TurnoutLinkableRailTile::updateVisible()
{
  Attributes::setVisible({outputMap, feedbackMap}, !linked);
  Attributes::setVisible({linkTurnout, linkInvert}, linked);
}

void TurnoutLinkableRailTile::syncPosition()
{
  assert(linkTurnout);
  updatePosition(Source::Link, convertPosition(*linkTurnout, *this, linkTurnout->position, linkInvert));
}

void TurnoutLinkableRailTile::unlinkTurnout(TurnoutLinkableRailTile& turnout)
{
  if(auto it = std::find(m_linkedTurnouts.begin(), m_linkedTurnouts.end(), turnout.shared_ptr<TurnoutLinkableRailTile>()); it != m_linkedTurnouts.end())
  {
    m_linkedTurnouts.erase(it);
    updateEnabled();
  }
}

TurnoutPosition TurnoutLinkableRailTile::convertPosition(const TurnoutLinkableRailTile& src, const TurnoutLinkableRailTile& dst, TurnoutPosition position, bool invert)
{
  if(position == TurnoutPosition::Unknown)
  {
    return TurnoutPosition::Unknown;
  }

  static constexpr std::array<TurnoutPosition, 3> positions{{
    TurnoutPosition::Straight,
    TurnoutPosition::Left,
    TurnoutPosition::Right,
  }};

  assert(std::find(positions.begin(), positions.end(), position) != positions.end());

  if(dst.isValidPosition(position))
  {
    if(!invert)
    {
      return position;
    }
    for(auto other : positions)
    {
      if(other != position && dst.isValidPosition(other))
      {
        return other;
      }
    }
  }
  else
  {
    for(auto other : positions)
    {
      if(invert == src.isValidPosition(other) && dst.isValidPosition(other))
      {
        return other;
      }
    }
  }

  assert(false);
  return TurnoutPosition::Unknown;
}
