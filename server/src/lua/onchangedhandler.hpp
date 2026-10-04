/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_LUA_ONCHANGEDHANDLER_HPP
#define TRAINTASTIC_SERVER_LUA_ONCHANGEDHANDLER_HPP

#include <memory>
#include <string>
#include <vector>
#include <boost/signals2/connection.hpp>
#include <lua.hpp>

class Object;
class BaseProperty;

namespace Lua {

class OnChangedHandler : public std::enable_shared_from_this<OnChangedHandler>
{
public:
  static constexpr char const* metaTableName = "on_changed";

  static OnChangedHandler& check(lua_State* L, int index);
  static OnChangedHandler* test(lua_State* L, int index);

  static void push(lua_State* L, OnChangedHandler& value);

  static void registerType(lua_State* L);

  OnChangedHandler(::Object& object, lua_State* L, int functionIndex);
  ~OnChangedHandler();

  inline lua_State* state()
  {
    return m_L;
  }

  void disconnect();
  void setFilter(std::vector<std::string> filter);

private:
  static int __gc(lua_State* L);
  static int __index(lua_State* L);
  static int disconnect(lua_State* L);
  static int pause(lua_State* L);
  static int resume(lua_State* L);

  lua_State* m_L;
  int m_function;
  int m_userData;
  boost::signals2::scoped_connection m_connection;
  std::vector<std::string> m_filter;
  bool m_paused = false;

  void propertyChanged(::BaseProperty& property);
  void release();
};

}

#endif
