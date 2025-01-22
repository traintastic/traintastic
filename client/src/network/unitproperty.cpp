/**
 * client/src/network/unitproperty.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2024 Reinder Feenstra
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

#include "unitproperty.hpp"
#include "connection.hpp"
#include "object.hpp"

UnitProperty::UnitProperty(Object& object, const QString& name, ValueType type, PropertyFlags flags, const QVariant& value, const QString& unitName, qint64 unitValue) :
  Property(object, name, type, flags, value),
  m_unitName{unitName},
  m_unitValue{unitValue}
{
}

void UnitProperty::setUnitValue(qint64 value)
{
  object().connection()->setUnitPropertyUnit(*this, value);
}
