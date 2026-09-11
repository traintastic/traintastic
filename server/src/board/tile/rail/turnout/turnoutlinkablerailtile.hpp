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

#ifndef TRAINTASTIC_SERVER_BOARD_TILE_RAIL_TURNOUT_TURNOUTLINKABLERAILTILE_HPP
#define TRAINTASTIC_SERVER_BOARD_TILE_RAIL_TURNOUT_TURNOUTLINKABLERAILTILE_HPP

#include "turnoutrailtile.hpp"

class TurnoutLinkableRailTile : public TurnoutRailTile
{
  friend struct TurnoutLinkableRailTileTestAccess;

public:
  Property<bool> linked;
  ObjectProperty<TurnoutLinkableRailTile> linkTurnout;
  Property<bool> linkInvert;

protected:
  TurnoutLinkableRailTile(World& world, std::string_view id_, TileId tileId_);

  void addToWorld() override;
  void loaded() override;
  void destroying() override;
  void worldEvent(WorldState state, WorldEvent event) override;

  bool doSetPosition(TurnoutPosition value, bool skipAction = false) override;
  void newPosition(TurnoutPosition value) override;

private:
  std::vector<std::shared_ptr<TurnoutLinkableRailTile>> m_linkedTurnouts;

  void linkedChanged();
  void updateEnabled();
  void updateVisible();

  void syncPosition();
  void unlinkTurnout(TurnoutLinkableRailTile& turnout);

  static TurnoutPosition convertPosition(const TurnoutLinkableRailTile& src, const TurnoutLinkableRailTile& dst, TurnoutPosition position, bool invert);
};

#endif
