/**
 * client/src/network/event.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_NETWORK_EVENT_HPP
#define TRAINTASTIC_CLIENT_NETWORK_EVENT_HPP

#include "interfaceitem.hpp"
#include <vector>
#include <traintastic/enum/valuetype.hpp>
#include "objectptr.hpp"

class Event : public InterfaceItem
{
  Q_OBJECT

  private:
    const std::vector<ValueType> m_argumentTypes;

  public:
    Event(Object& object, const QString& name, std::vector<ValueType> argumentTypes);

    const std::vector<ValueType>& argumentTypes() const { return m_argumentTypes; }

  signals:
    void fired(QVariantList arguments);
};

#endif
