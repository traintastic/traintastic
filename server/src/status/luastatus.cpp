/**
 * server/src/status/luastatus.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#include "luastatus.hpp"

LuaStatus::LuaStatus(Object& parent_, std::string_view parentPropertyName_)
  : Status(parent_, parentPropertyName_)
  , running(this, "running", 0, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::NoScript)
  , error(this, "error", 0, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::NoScript)
{
  label.setValueInternal("$world:lua_scripts$");

  m_interfaceItems.add(running);
  m_interfaceItems.add(error);
}
