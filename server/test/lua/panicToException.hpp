#ifndef TRAINTASTIC_SERVER_TEST_LUA_PANICTOEXCEPTION_HPP
#define TRAINTASTIC_SERVER_TEST_LUA_PANICTOEXCEPTION_HPP

#include <exception>
#include <lua.hpp>

class LuaPanicException : public std::runtime_error
{
  public:
    LuaPanicException(lua_State* L) :
      std::runtime_error(lua_tostring(L, -1))
    {
    }
};

inline int panicToException(lua_State* L)
{
  throw LuaPanicException(L);
}

#endif
