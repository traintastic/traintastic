/**
 * client/src/network/abstractproperty.hpp
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

#ifndef CLIENT_NETWORK_ABSTRACTPROPERTY_HPP
#define CLIENT_NETWORK_ABSTRACTPROPERTY_HPP

#include "interfaceitem.hpp"
#include <enum/propertytype.hpp>

class Object;

class AbstractProperty : public InterfaceItem
{
  Q_OBJECT

  protected:
    const PropertyType m_type;

  public:
    explicit AbstractProperty(Object& object, const QString& name, PropertyType type) :
      InterfaceItem(object, name),
      m_type{type}
    {
    }

    PropertyType type() const { return m_type; }

    virtual const QString& enumName() const = 0;

    virtual bool toBool() const = 0;
    virtual int toInt() const = 0;
    virtual int64_t toInt64() const = 0;
    virtual double toDouble() const = 0;
    virtual QString toString() const = 0;
    virtual QVariant toVariant() const = 0;

  signals:
    void valueChanged();
    void valueChangedBool(bool newValue);
    void valueChangedInt(int newValue);
    void valueChangedInt64(int64_t newValue);
    void valueChangedDouble(double newValue);
    void valueChangedString(const QString& newValue);

  public slots:
    virtual void setValueBool(bool value) = 0;
    virtual void setValueInt(int value) = 0;
    virtual void setValueInt64(int64_t value) = 0;
    virtual void setValueDouble(double value) = 0;
    virtual void setValueString(const QString& value) = 0;
};

#endif
