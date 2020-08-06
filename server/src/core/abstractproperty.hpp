/**
 * server/src/core/abstractproperty.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_CORE_ABSTRACTPROPERTY_HPP
#define TRAINTASTIC_SERVER_CORE_ABSTRACTPROPERTY_HPP

#include "interfaceitem.hpp"
#include <traintastic/enum/valuetype.hpp>
#include <traintastic/enum/propertyflags.hpp>
#include "objectptr.hpp"
#include <cassert>
#include "../utils/json.hpp"

class AbstractProperty : public InterfaceItem
{
  protected:
    const ValueType m_type;
    const PropertyFlags m_flags;

    void changed();

  public:
    AbstractProperty(Object& object, const std::string& name, ValueType type, PropertyFlags flags) :
      InterfaceItem{object, name},
      m_type{type},
      m_flags{flags}
    {
      assert(type != ValueType::Invalid);
      assert(is_access_valid(flags));
      assert(is_store_valid(flags));
    }

    bool isWriteable() const
    {
      return (m_flags & PropertyFlagsAccessMask) == PropertyFlags::ReadWrite;
    }

    bool isStoreable() const
    {
      return (m_flags & PropertyFlagsStoreMask) == PropertyFlags::Store;
    }

    PropertyFlags flags() const
    {
      return m_flags;
    }

    ValueType type() const
    {
      return m_type;
    }

    virtual std::string_view enumName() const = 0;
    virtual std::string_view setName() const = 0;

    virtual bool toBool() const = 0;
    virtual int64_t toInt64() const = 0;
    virtual double toDouble() const = 0;
    virtual std::string toString() const = 0;
    virtual ObjectPtr toObject() const = 0;
    virtual nlohmann::json toJSON() const = 0;

    virtual void fromBool(bool value) = 0;
    virtual void fromInt64(int64_t value) = 0;
    virtual void fromDouble(double value) = 0;
    virtual void fromString(const std::string& value) = 0;
    virtual void fromObject(const ObjectPtr& value) = 0;

    virtual void load(const nlohmann::json& value) = 0;
    virtual void load(const ObjectPtr& value) = 0;
};

#endif
