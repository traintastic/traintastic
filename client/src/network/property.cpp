/**
 * client/src/network/property.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019 Reinder Feenstra
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

#include "property.hpp"
#include "connection.hpp"
#include "object.hpp"

Property::Property(Object& object, const QString& name, ValueType type, PropertyFlags flags, const QVariant& value) :
  AbstractProperty(object, name, type, flags),
  m_value{value}
{
}

void Property::setValueBool(bool value)
{
  object().connection()->setPropertyBool(*this, value);
}

void Property::setValueInt(int value)
{
  object().connection()->setPropertyInt64(*this, value);
}

void Property::setValueInt64(int64_t value)
{
  object().connection()->setPropertyInt64(*this, value);
}

void Property::setValueDouble(double value)
{
  object().connection()->setPropertyDouble(*this, value);
}

void Property::setValueString(const QString& value)
{
  object().connection()->setPropertyString(*this, value);
}
