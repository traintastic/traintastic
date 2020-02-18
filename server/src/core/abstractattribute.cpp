/**
 * server/src/core/abstractattribute.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020 Reinder Feenstra
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

#include "abstractattribute.hpp"
#include "interfaceitem.hpp"
#include "object.hpp"

AbstractAttribute::AbstractAttribute(InterfaceItem& item, AttributeName name, ValueType type) :
  m_item{item},
  m_name{name},
  m_type{type}
{
}

void AbstractAttribute::changed()
{
  m_item.object().attributeChanged(*this);
}
