/**
 * client/src/network/property.hpp
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

#ifndef CLIENT_NETWORK_PROPERTY_HPP
#define CLIENT_NETWORK_PROPERTY_HPP

#include "interfaceitem.hpp"
#include <QVariant>
#include <enum/propertytype.hpp>
//#include "objectptr.hpp"

class Object;

class Property : public InterfaceItem
{
  Q_OBJECT

  friend class Client;

  protected:
    const PropertyType m_type;
    QVariant m_value;

  public:
    explicit Property(Object& object, const QString& name, PropertyType type, const QVariant& value);

    PropertyType type() const { return m_type; }

    bool toBool() const { return m_value.toBool(); }
    int64_t toInt64() const { return m_value.toLongLong(); }
    double toDouble() const { return m_value.toDouble(); }
    QString toString() const { return m_value.toString(); }
    QVariant toVariant() const { return m_value; }

  signals:
    void valueChanged();
    void valueChangedBool(bool newValue);
    void valueChangedInt64(int64_t newValue);
    void valueChangedDouble(double newValue);
    void valueChangedString(const QString& newValue);

  public slots:
    void setValueBool(bool value);
    void setValueInt64(int64_t value);
    void setValueDouble(double value);
    void setValueString(const QString& value);
};

#endif
