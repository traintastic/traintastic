/**
 * server/src/board/tile/misc/labeltile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024-2025 Reinder Feenstra
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

#include "labeltile.hpp"
#include "../../../core/attributes.hpp"
#include "../../../utils/category.hpp"

CREATE_IMPL(LabelTile)

LabelTile::LabelTile(World& world, std::string_view _id)
  : Tile(world, _id, TileId::Label)
  , backgroundColor{this, "background_color", Color::None, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadWrite}
  , text{this, "text", id, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadWrite}
  , textAlign{this, "text_align", TextAlign::Center, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadWrite}
  , textColor{this, "text_color", Color::None, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadWrite}
{
  Attributes::setMax<uint8_t>(height, 16);
  Attributes::setMax<uint8_t>(width, 16);

  m_interfaceItems.add(text);

  Attributes::addCategory(textAlign, Category::colorsAndAlignment);
  Attributes::addValues(textAlign, textAlignValues);
  m_interfaceItems.add(textAlign);

  Attributes::addCategory(textColor, Category::colorsAndAlignment);
  Attributes::addValues(textColor, colorValues);
  m_interfaceItems.add(textColor);

  Attributes::addCategory(backgroundColor, Category::colorsAndAlignment);
  Attributes::addValues(backgroundColor, colorValues);
  m_interfaceItems.add(backgroundColor);
}
