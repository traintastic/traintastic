/**
 * server/src/core/method.hpp
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

#ifndef TRAINTASTIC_SERVER_CORE_METHOD_HPP
#define TRAINTASTIC_SERVER_CORE_METHOD_HPP

#include "abstractmethod.hpp"
#include <functional>
#include <tuple>
#include "valuetypetraits.hpp"
#include "../utils/is_shared_ptr.hpp"

template<class T>
class Method;
//*
template<class... A>
struct args
{
  static constexpr std::size_t count = sizeof...(A);
  static constexpr std::array<ValueType, count> types = {{value_type_v<A>...}};
};
//*/
/*
template<class A1>
struct args
{
  static constexpr std::size_t count = 1;
  static constexpr std::array<ValueType, count> types = {value_type_v<A1>};
};
//*/

template<std::size_t N, class... A>
using getArgumentType = typename std::tuple_element<N, std::tuple<A...>>::type;

template<class R, class... A>
class Method<R(A...)> : public AbstractMethod
{
  private:

  protected:
    std::function<R(A...)> m_function;

  public:
    Method(Object& object, const std::string& name, std::function<R(A...)> function) :
      AbstractMethod(object, name),
      m_function{std::move(function)}
    {
    }

    R operator()(A... args)
    {
      return m_function(args...);
    }

    std::size_t argumentCount() const final
    {
      return sizeof...(A);
    }

    std::vector<ValueType> argumentTypes() const final
    {
      if constexpr(sizeof...(A) == 0)
        return {};
      else
        return std::vector<ValueType>(args<A...>::types.begin(), args<A...>::types.end());
    }

    ValueType resultType() const final
    {
      if constexpr(std::is_same_v<R, void>)
        return ValueType::Invalid;
      else if constexpr(value_type_v<R> != ValueType::Invalid)
        return value_type_v<R>;
      else if constexpr(is_shared_ptr_v<R> && std::is_base_of_v<Object, typename R::element_type>)
        return ValueType::Object;
      else
        static_assert(sizeof(R) != sizeof(R));
    }

    Result call(const std::vector<Argument>& args) final
    {
      if constexpr(std::is_same_v<R, void>)
      {
        if constexpr(sizeof...(A) == 0)
          m_function(/* some magic here */);
        else if constexpr(sizeof...(A) == 1)
        {
          if constexpr(std::is_same_v<getArgumentType<0, A...>, std::string>)
            m_function(std::get<std::string>(args[0]));
          else
            assert(false);
        }
        else
          static_assert(sizeof(R) != sizeof(R));
        //  m_function(args[0]);
        return Result();
      }
      else
      {
        if constexpr(sizeof...(A) == 0)
          return m_function(/* and here */);
        else if constexpr(sizeof...(A) == 1)
          return m_function(std::get<std::string>(args[0]));
        else if constexpr(sizeof...(A) == 4)
          return m_function(
            std::get<int64_t/*getArgumentType<0, A...>*/>(args[0]),
            std::get<int64_t/*getArgumentType<1, A...>*/>(args[1]),
            static_cast<getArgumentType<2, A...>>(std::get<int64_t/*getArgumentType<1, A...>*/>(args[2])),
            std::get<std::string/*getArgumentType<2, A...>*/>(args[3]));
        else
          static_assert(sizeof(R) != sizeof(R));
      }
    }
};

#endif
