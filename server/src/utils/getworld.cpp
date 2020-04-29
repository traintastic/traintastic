/**
 * server/src/utils/getworld.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

std::shared_ptr<World> getWorld(Object* object)
{
  if(IdObject* idObject = dynamic_cast<IdObject*>(object))
    return idObject->world().lock();
  else if(SubObject* subObject = dynamic_cast<SubObject*>(object))
    return getWorld(&subObject->parent());
  else if(World* world = dynamic_cast<World*>(object))
    return world->shared_ptr<World>();
  else
    return nullptr;
}
