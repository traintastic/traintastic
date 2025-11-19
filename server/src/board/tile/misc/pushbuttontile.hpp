/**
 * server/src/board/tile/misc/pushbuttontile.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_BOARD_TILE_MISC_PUSHBUTTONTILE_HPP
#define TRAINTASTIC_SERVER_BOARD_TILE_MISC_PUSHBUTTONTILE_HPP

#include "../tile.hpp"
#include "../../../core/method.hpp"
#include "../../../core/event.hpp"
#include "../../../enum/color.hpp"

class PushButtonTile : public Tile
{
  CLASS_ID("board_tile.misc.push_button")
  DEFAULT_ID("push_button")
  CREATE_DEF(PushButtonTile)

  public:
    Property<Color> color;
    Property<std::string> text;
    Property<Color> textColor;
    Method<void()> pressed;
    Event<std::shared_ptr<PushButtonTile>> onPressed;

    PushButtonTile(World& world, std::string_view _id);
};

#endif
