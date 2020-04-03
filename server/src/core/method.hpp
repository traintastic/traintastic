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
#include "../utils/is_shared_ptr.hpp"

template<class T>
class Method;

template<class... A>
struct args
{
  static constexpr std::size_t count = sizeof...(A);
  static constexpr std::array<ValueType, count> types = {};
};

template<class R, class... A>
class Method<R(A...)> : public AbstractMethod
{
  protected:
    std::function<R(A...)> m_function;

  public:
    Method(Object& object, const std::string& name, std::function<R(A...)> function) :
      AbstractMethod(object, name),
      m_function{function}
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
        m_function(/* some magic here */);
        return Result();
      }
      else
        return m_function(/* and here */);
    }
};

#endif
