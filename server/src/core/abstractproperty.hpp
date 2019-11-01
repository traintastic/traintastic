/**
 * server/src/core/abstractproperty.hpp
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

#ifndef SERVER_CORE_ABSTRACTPROPERTY_HPP
#define SERVER_CORE_ABSTRACTPROPERTY_HPP

#include "interfaceitem.hpp"
#include <enum/propertytype.hpp>
#include "propertyflags.hpp"
#include "objectptr.hpp"
#include <cassert>
#include <nlohmann/json.hpp>

class AbstractProperty : public InterfaceItem
{
  protected:
    const PropertyType m_type;
    PropertyFlags m_flags;

    void changed();

  public:
    AbstractProperty(Object& object, const std::string& name, PropertyType type, PropertyFlags flags) :
      InterfaceItem{object, name},
      m_type{type},
      m_flags{flags}
    {
      assert(type != PropertyType::Invalid);
      assert(is_access_valid(flags) && is_store_valid(flags));
    }

    bool isConstant() const
    {
      return false;//!is_empty(m_flags & PropertyFlags::Constant);
    }

    bool isReadable() const
    {
      return true;//!is_empty(m_flags & (PropertyFlags::Constant | PropertyFlags::ReadOnly));
    }

    bool isWriteable() const
    {
      return true;//!is_empty(m_flags & PropertyFlags::WriteOnly);
    }

    PropertyType type() const
    {
      return m_type;
    }

    virtual bool toBool() const = 0;
    virtual int64_t toInt64() const = 0;
    virtual double toDouble() const = 0;
    virtual std::string toString() const = 0;
    virtual ObjectPtr toObject() const = 0;

    virtual void fromBool(bool value) = 0;
    virtual void fromInt64(int64_t value) = 0;
    virtual void fromDouble(double value) = 0;
    virtual void fromString(const std::string& value) = 0;
    virtual void fromObject(const ObjectPtr& value) = 0;

    nlohmann::json toJSON() const
    {
      switch(type())
      {
        case PropertyType::Boolean:
          return toBool();

        case PropertyType::Integer:
          return toInt64();

        case PropertyType::Float:
          return toDouble();

        case PropertyType::String:
          return toString();

        // TODO: case nlohmann::json::value_t::object:
        // TODO: case nlohmann::json::value_t::array:

        default:
          throw std::runtime_error("unsupported type");
      }
    }

    void fromJSON(const nlohmann::json& value)
    {
      switch(value.type())
      {
        // TODO: case nlohmann::json::value_t::null:

        case nlohmann::json::value_t::boolean:
          fromBool(value);
          break;

        case nlohmann::json::value_t::number_integer:
        case nlohmann::json::value_t::number_unsigned:
          fromInt64(value);
          break;

        case nlohmann::json::value_t::number_float:
          fromDouble(value);
          break;

        case nlohmann::json::value_t::string:
          fromString(value);
          break;

        // TODO: case nlohmann::json::value_t::object:
        // TODO: case nlohmann::json::value_t::array:

        default:
          throw std::runtime_error("unsupported JSON type");
      }
    }
};

#endif
