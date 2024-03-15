/**
 * client/src/network/objectvectorproperty.cpp
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

#include "objectvectorproperty.hpp"
#include "object.hpp"
#include "connection.hpp"

int ObjectVectorProperty::getObject(int index, std::function<void(const ObjectPtr&, std::optional<const Error>)> callback)
{
  assert(index >= 0 && index < size());
  return object().connection()->getObject(*this, static_cast<uint32_t>(index), std::move(callback));
}

int ObjectVectorProperty::getObjects(int startIndex, int endIndex, std::function<void(const std::vector<ObjectPtr>&, std::optional<const Error>)> callback)
{
  assert(startIndex >= 0 && startIndex < size());
  assert(endIndex >= 0 && endIndex < size());
  assert(endIndex >= startIndex);
  return object().connection()->getObjects(*this, static_cast<uint32_t>(startIndex), static_cast<uint32_t>(endIndex), std::move(callback));
}
