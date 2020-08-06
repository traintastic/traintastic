/**
 * shared/src/enum/enum.hpp
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


#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_ENUM_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_ENUM_HPP

#include <frozen/map.h>

#define ENUM_NAME(_type, _name) \
  template<> \
  struct EnumName<_type> \
  { \
    static constexpr char const* value = _name; \
  };

#define ENUM_VALUES(_type, _size, ...) \
  template<> \
  struct EnumValues<_type> \
  { \
    static constexpr frozen::map<_type, const char*, _size> value = { __VA_ARGS__ }; \
  };

template<typename T>
struct EnumName
{
  static_assert(sizeof(T) != sizeof(T), "template specialization required");
};

template<typename T>
struct EnumValues
{
  static_assert(sizeof(T) != sizeof(T), "template specialization required");
};

#endif
