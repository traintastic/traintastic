/**
 * server/src/core/methodflags.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_CORE_METHODFLAGS_HPP
#define TRAINTASTIC_SERVER_CORE_METHODFLAGS_HPP

enum class MethodFlags
{
  // bit 0..1
  NoScript = 1 << 0,
  ScriptCallable = 2 << 0,

  // bit 2
  Internal = 1 << 2,
};

constexpr MethodFlags operator |(MethodFlags lhs, MethodFlags rhs)
{
  using T = std::underlying_type_t<MethodFlags>;
  return static_cast<MethodFlags>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

constexpr MethodFlags operator &(MethodFlags lhs, MethodFlags rhs)
{
  using T = std::underlying_type_t<MethodFlags>;
  return static_cast<MethodFlags>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

/// temporary placeholder, should be removed in the future when all method have their flags set
constexpr MethodFlags noMethodFlags = static_cast<MethodFlags>(0);

#endif
