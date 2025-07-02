/**
 * server/src/throttle/webthrottle.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2025 Reinder Feenstra
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

#include "webthrottle.hpp"
#include "../world/world.hpp"

std::shared_ptr<WebThrottle> WebThrottle::create(World& world)
{
  auto obj = std::make_shared<WebThrottle>(world, getUniqueLogId("webthrottle"));
  obj->addToList();
  return obj;
}

WebThrottle::WebThrottle(World& world, std::string_view logId)
  : Throttle(world, logId)
{
}
