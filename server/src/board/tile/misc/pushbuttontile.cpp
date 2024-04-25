/**
 * server/src/board/tile/misc/pushbuttontile.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022,2024 Reinder Feenstra
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
#include "../../../utils/displayname.hpp"

CREATE_IMPL(PushButtonTile)

PushButtonTile::PushButtonTile(World& world, std::string_view _id)
  : Tile(world, _id, TileId::PushButton)
  , name{this, "name", id, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly}
  , color{this, "color", Color::Yellow, PropertyFlags::ReadWrite | PropertyFlags::Store | PropertyFlags::ScriptReadOnly}
  , pressed{*this, "pressed",
      [this]()
      {
        fireEvent(onPressed, shared_ptr<PushButtonTile>());
      }}
  , onPressed{*this, "on_pressed", EventFlags::Scriptable}
{
  const bool editable = contains(m_world.state.value(), WorldState::Edit);

  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);

  Attributes::addEnabled(color, editable);
  Attributes::addValues(color, colorValuesWithoutNone);
  m_interfaceItems.add(color);

  Attributes::addObjectEditor(pressed, false);
  m_interfaceItems.add(pressed);

  m_interfaceItems.add(onPressed);
}

void PushButtonTile::worldEvent(WorldState worldState, WorldEvent worldEvent)
{
  Tile::worldEvent(worldState, worldEvent);

  const bool editable = contains(worldState, WorldState::Edit);

  Attributes::setEnabled(name, editable);
  Attributes::setEnabled(color, editable);
}
