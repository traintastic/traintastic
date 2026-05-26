/**
 * client/src/network/interfaceitems.hpp
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

#ifndef TRAINTASTIC_CLIENT_NETWORK_INTERFACEITEMS_HPP
#define TRAINTASTIC_CLIENT_NETWORK_INTERFACEITEMS_HPP

#include <QMap>
#include <QStringList>

class InterfaceItem;

class InterfaceItems
{
  protected:
    QMap<QString, InterfaceItem*> m_items;
    QStringList m_itemOrder;

  public:
    const QStringList& names() const { return m_itemOrder; }

    QStringList categories() const;

    std::vector<InterfaceItem*> items() const;
    std::vector<InterfaceItem*> items(const QString& category) const;

    inline InterfaceItem* find(const QString& name) const { return m_items.value(name, nullptr); }

    void add(InterfaceItem& item);
};

#endif
