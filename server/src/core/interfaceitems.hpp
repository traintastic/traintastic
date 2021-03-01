/**
 * server/src/core/interfaceitems.hpp
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

#ifndef TRAINTASTIC_SERVER_CORE_INTERFACEITEMS_HPP
#define TRAINTASTIC_SERVER_CORE_INTERFACEITEMS_HPP

#include <unordered_map>
#include <list>
#include <string_view>

class InterfaceItem;

class InterfaceItems
{
  protected:
    std::unordered_map<std::string_view, InterfaceItem&> m_items;
    std::list<std::string_view> m_itemOrder;

  public:
    using const_iterator = std::unordered_map<std::string_view, InterfaceItem&>::const_iterator;

    inline const_iterator begin() const { return m_items.cbegin(); }
    inline const_iterator end() const { return m_items.cend(); }

    const std::list<std::string_view>& names() const { return m_itemOrder; }

    InterfaceItem* find(std::string_view name) const;

    void add(InterfaceItem& item);
    void insertBefore(InterfaceItem& item, const InterfaceItem& before);

    inline InterfaceItem& operator[](std::string_view name) const { return m_items.at(name); }
};

#endif
