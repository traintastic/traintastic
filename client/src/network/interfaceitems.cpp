/**
 * client/src/network/interfaceitems.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2025 Reinder Feenstra
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

QStringList InterfaceItems::categories() const
{
  QStringList list;
  for (const auto* item : items())
  {
    if(auto category = item->getAttributeString(AttributeName::Category, {}); !list.contains(category))
    {
      list.append(category);
    }
  }
  return list;
}

std::vector<InterfaceItem*> InterfaceItems::items() const
{
  std::vector<InterfaceItem*> vec;
  vec.reserve(m_itemOrder.size());
  for (const auto& name : m_itemOrder)
  {
    if(auto* item = find(name)) [[likely]]
    {
      vec.emplace_back(item);
    }
  }
  return vec;
}

std::vector<InterfaceItem*> InterfaceItems::items(const QString& category) const
{
  auto vec = items();
  for(auto it = vec.begin(); it < vec.end();)
  {
    if((*it)->getAttributeString(AttributeName::Category, {}) == category)
    {
      it++;
    }
    else
    {
      it = vec.erase(it);
    }
  }
  return vec;
}

void InterfaceItems::add(InterfaceItem& item)
{
  m_items.insert(item.name(), &item);
  m_itemOrder.append(item.name());
}
