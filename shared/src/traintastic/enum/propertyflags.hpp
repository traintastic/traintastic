/**
 * server/src/core/PropertyFlags.hpp
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

#ifndef SERVER_CORE_PropertyFlags_HPP
#define SERVER_CORE_PropertyFlags_HPP

#include <cstdint>

enum class PropertyFlags : uint16_t
{
  // bit 0..1
  ReadOnly = 1 << 0,
  ReadWrite = PropertyFlags::ReadOnly | 2 << 0,

  // bit 2..4
  NoStore = 1 << 2,
  Store = 2 << 2,
  StoreState = 3 << 2,

  // bit 5
  SubObject = 1 << 4

/*

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



  TODO = AccessWWW,
  */
};

constexpr PropertyFlags operator| (const PropertyFlags& lhs, const PropertyFlags& rhs)
{
  return static_cast<PropertyFlags>(static_cast<uint16_t>(lhs) | static_cast<uint16_t>(rhs));
}

constexpr PropertyFlags& operator|= (PropertyFlags& lhs, const PropertyFlags& rhs)
{
  return lhs = lhs | rhs;
}

constexpr PropertyFlags operator& (const PropertyFlags& lhs, const PropertyFlags& rhs)
{
  return static_cast<PropertyFlags>(static_cast<uint16_t>(lhs) & static_cast<uint16_t>(rhs));
}

constexpr PropertyFlags& operator&= (PropertyFlags& lhs, const PropertyFlags& rhs)
{
  return lhs = lhs & rhs;
}

constexpr bool contains(const PropertyFlags value, const PropertyFlags mask)
{
  return (value & mask) == mask;
}

constexpr bool is_empty(const PropertyFlags value)
{
  return value == static_cast<PropertyFlags>(0x0000);
}

constexpr PropertyFlags PropertyFlagsAccessMask = static_cast<PropertyFlags>(0x0003);
constexpr PropertyFlags PropertyFlagsStoreMask = static_cast<PropertyFlags>(0x000C);

constexpr bool is_access_valid(const PropertyFlags value)
{
  return !is_empty(value & PropertyFlagsAccessMask);
}

constexpr bool is_store_valid(const PropertyFlags /*value*/)
{
  return true;
  //const PropertyFlags store = value & PropertyFlagsStoreMask;
  //return (store == PropertyFlags::NoStore) || (store == PropertyFlags::Store) || (store == PropertyFlags::StoreState);
}

#endif
