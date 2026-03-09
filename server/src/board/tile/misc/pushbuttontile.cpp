/**
 * server/src/board/tile/misc/pushbuttontile.cpp
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

#include "pushbuttontile.hpp"
#include "../../../world/world.hpp"
#include "../../../core/method.tpp"
#include "../../../core/attributes.hpp"
#include "../../../utils/category.hpp"
#include "../../../utils/displayname.hpp"

CREATE_IMPL(PushButtonTile)

PushButtonTile::PushButtonTile(World& world, std::string_view _id)
  : Tile(world, _id, TileId::PushButton)
  , color{this, "color", Color::Yellow, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadWrite}
  , text{this, "text", "", PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadWrite}
  , textColor{this, "text_color", Color::Black, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadWrite}
  , pressed{*this, "pressed",
      [this]()
      {
        fireEvent(onPressed, shared_ptr<PushButtonTile>());
      }}
  , onPressed{*this, "on_pressed", EventFlags::Scriptable}
{
  Attributes::setMax<uint8_t>(height, 16);
  Attributes::setMax<uint8_t>(width, 16);

  Attributes::addCategory(color, Category::colors);
  Attributes::addValues(color, colorValuesWithoutNone);
  m_interfaceItems.add(color);

  m_interfaceItems.add(text);

  Attributes::addCategory(textColor, Category::colors);
  Attributes::addValues(textColor, colorValuesWithoutNone);
  m_interfaceItems.add(textColor);

  Attributes::addObjectEditor(pressed, false);
  m_interfaceItems.add(pressed);

  m_interfaceItems.add(onPressed);
}
