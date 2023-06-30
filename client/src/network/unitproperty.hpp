/**
 * client/src/network/unitproperty.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_NETWORK_UNITPROPERTY_HPP
#define TRAINTASTIC_CLIENT_NETWORK_UNITPROPERTY_HPP

#include "property.hpp"

template<typename T> struct EnumName;

class UnitProperty : public Property
{
  Q_OBJECT

  friend class Connection;

  protected:
    QVariant m_value;
    const QString m_unitName;
    qint64 m_unitValue;

  public:
    UnitProperty(Object& object, const QString& name, ValueType type, PropertyFlags flags, const QVariant& value, const QString& unitName, qint64 unitValue);

    const QString& unitName() const { return m_unitName; }
    qint64 unitValue() const { return m_unitValue; }

    template<typename T>
    T unit() const
    {
      static_assert(std::is_enum_v<T>);
      assert(EnumName<T>::value == m_unitName);
      return static_cast<T>(unitValue());
    }

    void setUnitValue(qint64 value);
};

#endif
