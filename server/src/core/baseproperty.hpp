/**
 * server/src/core/baseproperty.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_CORE_BASEPROPERTY_HPP
#define TRAINTASTIC_SERVER_CORE_BASEPROPERTY_HPP

#include "interfaceitem.hpp"
#include <traintastic/enum/valuetype.hpp>
#include <traintastic/enum/propertyflags.hpp>
#include <cassert>
#include "../utils/json.hpp"

class BaseProperty : public InterfaceItem
{
  protected:
    const ValueType m_type;
    const PropertyFlags m_flags;

    BaseProperty(Object& object, std::string_view name, ValueType type, PropertyFlags flags) :
      InterfaceItem{object, name},
      m_type{type},
      m_flags{flags}
    {
      assert(type != ValueType::Invalid);
      assert(is_access_valid(flags));
      assert(is_store_valid(flags));
      assert(isScriptValid(flags));
    }

    void changed();

  public:
    bool isWriteable() const
    {
      return (m_flags & PropertyFlagsAccessMask) == PropertyFlags::ReadWrite;
    }

    bool isStoreable() const
    {
      return (m_flags & PropertyFlagsStoreMask) == PropertyFlags::Store;
    }

    bool isStateStoreable() const
    {
      return (m_flags & PropertyFlagsStoreMask) == PropertyFlags::StoreState;
    }

    bool isScriptReadable() const
    {
      const PropertyFlags scriptFlags = m_flags & PropertyFlagsScriptMask;
      return scriptFlags == PropertyFlags::ScriptReadOnly || scriptFlags == PropertyFlags::ScriptReadWrite;
    }

    bool isScriptWriteable() const
    {
      return (m_flags & PropertyFlagsScriptMask) == PropertyFlags::ScriptReadWrite;
    }

    bool isInternal() const final
    {
      return (m_flags & PropertyFlags::Internal) == PropertyFlags::Internal;
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

    virtual nlohmann::json toJSON() const = 0;
};

#endif
