/**
 * server/src/lua/to.hpp
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

#ifndef TRAINTASTIC_SERVER_LUA_TO_HPP
#define TRAINTASTIC_SERVER_LUA_TO_HPP

#include <lua.hpp>
#include <type_traits>
#include <string>
#include <string_view>
#include "enum.hpp"
#include "set.hpp"

namespace Lua {

//! @brief Try to convert the Lua value at the given index to T
//! @param[in] L Lua state
//! @param[in] index Stack index
//! @param[out] value Converted value
//! @return @c true if successful, @c false otherwise
//! @note Because Lua has garbage collection, there is no guarantee that the @c std::string_view will be valid after the corresponding Lua value is removed from the stack.
//!
template<typename T>
bool to(lua_State* L, int index, T& value)
{
  if constexpr(std::is_same_v<T, bool>)
  {
    if(lua_isboolean(L, index))
    {
      value = lua_toboolean(L, index);
      return true;
    }
  }
  else if constexpr(is_set_v<T>)
  {
    return Set<T>::test(L, index, value);
  }
  else if constexpr(std::is_enum_v<T>)
  {
    return Enum<T>::test(L, index, value);
  }
  else if constexpr(std::is_integral_v<T>)
  {
    int isInt;
    lua_Integer v = lua_tointegerx(L, index, &isInt);
    if(isInt)
    {
      static_assert(std::is_signed_v<lua_Integer>);

      if constexpr(std::is_unsigned_v<T> && sizeof(T) >= sizeof(v))
      {
        if(v >= 0)
        {
          value = static_cast<T>(v);
          return true;
        }
      }
      else if constexpr(std::numeric_limits<T>::min() <= LUA_MININTEGER && std::numeric_limits<T>::max() >= LUA_MAXINTEGER)
      {
        value = v;
        return true;
      }
      else if(v >= std::numeric_limits<T>::min() && v <= std::numeric_limits<T>::max())
      {
        value = v;
        return true;
      }
    }
  }
  else if constexpr(std::is_floating_point_v<T>)
  {
    int isNum;
    lua_Number v = lua_tonumberx(L, index, &isNum);
    if(isNum)
    {
      value = static_cast<T>(v);
      return true;
    }
  }
  else if constexpr(std::is_same_v<T, std::string_view> || std::is_same_v<T, std::string>)
  {
    size_t len;
    if(const char* s = lua_tolstring(L, index, &len))
    {
      if constexpr(std::is_same_v<T, std::string_view>)
        value = T{s, len};
      else if constexpr(std::is_same_v<T, std::string>)
        value.assign(s, len);
      else
        static_assert(sizeof(T) != sizeof(T), "don't know how to assign");
      return true;
    }
  }
  else
    static_assert(sizeof(T) != sizeof(T), "don't know how to convert type");

  return false;
}

//! \brief Converts the Lua value at the given index to T
//! @param[in] L Lua state
//! @param[in] index Stack index
//! @return Converted value if successful, default value otherwise
//! @note Because Lua has garbage collection, there is no guarantee that the @c std::string_view will be valid after the corresponding Lua value is removed from the stack.
//!
template<typename T>
T to(lua_State* L, int index)
{
  if constexpr(std::is_same_v<T, bool>)
  {
    return lua_isboolean(L, index) && lua_toboolean(L, index);
  }
  else if constexpr(is_set_v<T>)
  {
    T v;
    return Set<T>::test(L, index, v) ? v : T();
  }
  else if constexpr(std::is_enum_v<T>)
  {
    T v;
    return Enum<T>::test(L, index, v) ? v : EnumValues<T>::value.begin()->first;
  }
  else if constexpr(std::is_integral_v<T> || std::is_floating_point_v<T>)
  {
    T v;
    return to<T>(L, index, v) ? v : 0;
  }
  else if constexpr(std::is_same_v<T, std::string_view> || std::is_same_v<T, std::string>)
  {
    size_t len;
    const char* s = lua_tolstring(L, index, &len);
    return T{s, len};
  }
  else
    static_assert(sizeof(T) != sizeof(T), "don't know how to convert type");
}

}

#endif
