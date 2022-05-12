/**
 * client/src/network/baseproperty.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_NETWORK_BASEPROPERTY_HPP
#define TRAINTASTIC_CLIENT_NETWORK_BASEPROPERTY_HPP

#include "interfaceitem.hpp"
#include <traintastic/enum/valuetype.hpp>
#include <traintastic/enum/propertyflags.hpp>

class Object;

class BaseProperty : public InterfaceItem
{
  Q_OBJECT

  private:
    inline static const QString enumOrSetNameEmpty;

  protected:
    const ValueType m_type;
    const PropertyFlags m_flags;

    BaseProperty(Object& object, const QString& name, ValueType type, PropertyFlags flags) :
      InterfaceItem(object, name),
      m_type{type},
      m_flags{flags}
    {
    }

  public:
    ValueType type() const { return m_type; }
    PropertyFlags flags() const { return m_flags; }

    bool isWritable() const { return (m_flags & PropertyFlagsAccessMask) == PropertyFlags::ReadWrite; }

    virtual const QString& enumName() const { Q_ASSERT(false); return enumOrSetNameEmpty; }
    virtual const QString& setName() const  { Q_ASSERT(false); return enumOrSetNameEmpty; }

  signals:
    void valueChanged();
};

#endif
