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

#ifndef TRAINTASTIC_CLIENT_NETWORK_ABSTRACTPROPERTY_HPP
#define TRAINTASTIC_CLIENT_NETWORK_ABSTRACTPROPERTY_HPP

#include "interfaceitem.hpp"
#include <enum/valuetype.hpp>
#include <enum/propertyflags.hpp>

class Object;

class AbstractProperty : public InterfaceItem
{
  Q_OBJECT

  private:
    inline static const QString enumNameEmpty;

  protected:
    const ValueType m_type;
    const PropertyFlags m_flags;

  public:
    explicit AbstractProperty(Object& object, const QString& name, ValueType type, PropertyFlags flags) :
      InterfaceItem(object, name),
      m_type{type},
      m_flags{flags}
    {
    }

    ValueType type() const { return m_type; }
    PropertyFlags flags() const { return m_flags; }

    bool isWritable() const { return (m_flags & PropertyFlagsAccessMask) == PropertyFlags::ReadWrite; }

    virtual const QString& enumName() const       { Q_ASSERT(false); return enumNameEmpty; }

    virtual bool toBool() const            { Q_ASSERT(false); return false; }
    virtual int toInt() const              { Q_ASSERT(false); return 0; }
    virtual int64_t toInt64() const        { Q_ASSERT(false); return 0; }
    virtual double toDouble() const        { Q_ASSERT(false); return 0; }
    virtual QString toString() const       { Q_ASSERT(false); return ""; }
    virtual QVariant toVariant() const     { Q_ASSERT(false); return QVariant(); }

  signals:
    void valueChanged();
    void valueChangedBool(bool newValue);
    void valueChangedInt(int newValue);
    void valueChangedInt64(int64_t newValue);
    void valueChangedDouble(double newValue);
    void valueChangedString(const QString& newValue);

  public slots:
    virtual void setValueBool(bool value)      { Q_ASSERT(value != value); }
    virtual void setValueInt(int value)          { Q_ASSERT(value != value); }
    virtual void setValueInt64(int64_t value)          { Q_ASSERT(value != value); }
    virtual void setValueDouble(double value)              { Q_ASSERT(value != value); }
    virtual void setValueString(const QString& value)  { Q_ASSERT(value != value); }
};

#endif
