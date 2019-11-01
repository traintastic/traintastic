/**
 * server/src/core/propertyflags.hpp
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

#ifndef SERVER_CORE_PROPERTYFLAGS_HPP
#define SERVER_CORE_PROPERTYFLAGS_HPP

#include <cstdint>

enum class PropertyFlags : uint32_t
{
  // Access:
  EditConstant = 1 << 0,
  EditReadOnly = 2 << 0,
  EditReadWrite = 3 << 0,
  StopConstant = 1 << 2,
  StopReadOnly = 2 << 2,
  StopReadWrite = 3 << 2,
  RunConstant = 1 << 4,
  RunReadOnly = 2 << 4,
  RunReadWrite = 3 << 4,

  AccessRRR = PropertyFlags::EditReadOnly | PropertyFlags::StopReadOnly | PropertyFlags::RunReadOnly,
  AccessRRW = PropertyFlags::EditReadOnly | PropertyFlags::StopReadOnly | PropertyFlags::RunReadWrite,
  AccessRWW = PropertyFlags::EditReadOnly | PropertyFlags::StopReadWrite | PropertyFlags::RunReadWrite,
  AccessWCC = PropertyFlags::EditReadWrite | PropertyFlags::StopConstant | PropertyFlags::RunConstant,
  AccessWWW = PropertyFlags::EditReadWrite | PropertyFlags::StopReadWrite | PropertyFlags::RunReadWrite,
  // Store:
  NoStore = 1 << 6,
  Store = 2 << 6,
  StoreState = 3 << 6,
};

constexpr PropertyFlags operator| (const PropertyFlags& lhs, const PropertyFlags& rhs)
{
  return static_cast<PropertyFlags>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr PropertyFlags& operator|= (PropertyFlags& lhs, const PropertyFlags& rhs)
{
  return lhs = lhs | rhs;
}

constexpr PropertyFlags operator& (const PropertyFlags& lhs, const PropertyFlags& rhs)
{
  return static_cast<PropertyFlags>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr PropertyFlags& operator&= (PropertyFlags& lhs, const PropertyFlags& rhs)
{
  return lhs = lhs & rhs;
}

constexpr bool is_empty(const PropertyFlags value)
{
  return value == static_cast<PropertyFlags>(0);
}

//constexpr PropertyFlags PropertyFlagsAccessMask = PropertyFlags::Constant | PropertyFlags::ReadOnly | PropertyFlags::WriteOnly;
//constexpr PropertyFlags PropertyFlagsStoreMask = PropertyFlags::NoStore | PropertyFlags::Store | PropertyFlags::StoreState;

constexpr bool is_access_valid(const PropertyFlags value)
{
  return true;
  //const PropertyFlags access = value & PropertyFlagsAccessMask;
  //return (access == PropertyFlags::Constant) || !is_empty(access & PropertyFlags::ReadWrite);
}

constexpr bool is_store_valid(const PropertyFlags value)
{
  return true;
  //const PropertyFlags store = value & PropertyFlagsStoreMask;
  //return (store == PropertyFlags::NoStore) || (store == PropertyFlags::Store) || (store == PropertyFlags::StoreState);
}

#endif
