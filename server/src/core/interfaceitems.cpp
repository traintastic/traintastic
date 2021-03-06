/**
 * server/src/core/interfaceitems.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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

#include "interfaceitems.hpp"
#include "interfaceitem.hpp"
#include <algorithm>

#ifndef NDEBUG
#include "abstractproperty.hpp"

static void check(const InterfaceItem& item)
{
  if(const AbstractProperty* property = dynamic_cast<const AbstractProperty*>(&item))
  {
    if(property->type() == ValueType::Enum)
      assert(property->attributes().find(AttributeName::Values) != property->attributes().cend()); // enum property must have a Values attribute
  }
}
#endif

InterfaceItem* InterfaceItems::find(std::string_view name) const
{
  auto it = m_items.find(name);
  return (it != m_items.end()) ? &it->second : nullptr;
}

void InterfaceItems::add(InterfaceItem& item)
{
#ifndef NDEBUG
  check(item);
#endif
  m_items.emplace(item.name(), item);
  m_itemOrder.push_back(item.name());
}

void InterfaceItems::insertBefore(InterfaceItem& item, const InterfaceItem& before)
{
#ifndef NDEBUG
  check(item);
#endif
  m_items.emplace(item.name(), item);
  m_itemOrder.insert(std::find(m_itemOrder.begin(), m_itemOrder.end(), before.name()), item.name());
}
