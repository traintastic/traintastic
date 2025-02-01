/**
 * server/src/utils/getworld.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2022 Reinder Feenstra
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

#include "getworld.hpp"
#include "../core/idobject.hpp"
#include "../core/subobject.hpp"

World& getWorld(Object* object)
{
  assert(object);
  if(auto* idObject = dynamic_cast<IdObject*>(object))
    return idObject->world();
  if(auto* subObject = dynamic_cast<SubObject*>(object))
    return getWorld(&subObject->parent());
  if(auto* world = dynamic_cast<World*>(object))
    return *world;
  assert(false);
  abort();
}
