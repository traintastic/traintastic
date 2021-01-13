/**
 * server/src/core/method.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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
#include <limits>
#include "valuetypetraits.hpp"
#include "../utils/is_shared_ptr.hpp"

template<class T>
class Method;

template<class... A>
struct args
{
  static constexpr std::size_t count = sizeof...(A);
  static constexpr std::array<ValueType, count> types = {{value_type_v<A>...}};
};

template<std::size_t N, class... A>
using getArgumentType = typename std::tuple_element<N, std::tuple<A...>>::type;

template<std::size_t N, class... A>
auto getArgument(const AbstractMethod::Argument& value)
{
  using T = std::remove_const_t<std::remove_reference_t<getArgumentType<N, A...>>>;

  if constexpr(std::is_same_v<T, bool>)
    return std::get<bool>(value);
  else if constexpr(std::is_enum_v<T>)
    return static_cast<T>(std::get<int64_t>(value)); // TODO: test if enum value is valid
  else if constexpr(std::is_integral_v<T>)
  {
    const int64_t r = std::get<int64_t>(value);
    if(r >= std::numeric_limits<T>::min() && r <= std::numeric_limits<T>::max())
      return static_cast<T>(r);
    else
      throw AbstractMethod::OutOfRangeArgumentError(N);
  }
  else if constexpr(std::is_floating_point_v<T>)
    return static_cast<T>(std::get<double>(value));
  else if constexpr(std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>)
    return std::get<std::string>(value);
  else if constexpr(is_shared_ptr_v<T>)
  {
    ObjectPtr obj = std::get<ObjectPtr>(value);
    if(!obj)
      return T();
    else if(auto r = std::dynamic_pointer_cast<typename T::element_type>(obj))
      return r;
    else
      throw AbstractMethod::InvalidObjectArgumentError(N);
  }
  else
    static_assert(sizeof(T) != sizeof(T));
}

template<class T>
constexpr AbstractMethod::Result toResult(const T& value)
{
  if constexpr(std::is_enum_v<T>)
    return static_cast<int64_t>(value);
  else
    return value;
}

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
      if(args.size() != argumentCount())
        throw InvalidNumberOfArgumentsError();

      if constexpr(std::is_same_v<R, void>)
      {
        if constexpr(sizeof...(A) == 0)
          m_function();
        else if constexpr(sizeof...(A) == 1)
          m_function(
            getArgument<0, A...>(args[0]));
        else if constexpr(sizeof...(A) == 2)
          m_function(
            getArgument<0, A...>(args[0]),
            getArgument<1, A...>(args[1]));
        else if constexpr(sizeof...(A) == 3)
          m_function(
            getArgument<0, A...>(args[0]),
            getArgument<1, A...>(args[1]),
            getArgument<2, A...>(args[2]));
        else if constexpr(sizeof...(A) == 4)
          m_function(
            getArgument<0, A...>(args[0]),
            getArgument<1, A...>(args[1]),
            getArgument<2, A...>(args[2]),
            getArgument<3, A...>(args[3]));
        else if constexpr(sizeof...(A) == 5)
          m_function(
            getArgument<0, A...>(args[0]),
            getArgument<1, A...>(args[1]),
            getArgument<2, A...>(args[2]),
            getArgument<3, A...>(args[3]),
            getArgument<4, A...>(args[4]));
        else
          static_assert(sizeof(R) != sizeof(R));

        return Result();
      }
      else
      {
        if constexpr(sizeof...(A) == 0)
          return toResult(m_function());
        else if constexpr(sizeof...(A) == 1)
          return toResult(m_function(
            getArgument<0, A...>(args[0])));
        else if constexpr(sizeof...(A) == 2)
          return toResult(m_function(
            getArgument<0, A...>(args[0]),
            getArgument<1, A...>(args[1])));
        else if constexpr(sizeof...(A) == 3)
          return toResult(m_function(
            getArgument<0, A...>(args[0]),
            getArgument<1, A...>(args[1]),
            getArgument<2, A...>(args[2])));
        else if constexpr(sizeof...(A) == 4)
          return toResult(m_function(
            getArgument<0, A...>(args[0]),
            getArgument<1, A...>(args[1]),
            getArgument<2, A...>(args[2]),
            getArgument<3, A...>(args[3])));
        else if constexpr(sizeof...(A) == 5)
          return toResult(m_function(
            getArgument<0, A...>(args[0]),
            getArgument<1, A...>(args[1]),
            getArgument<2, A...>(args[2]),
            getArgument<3, A...>(args[3]),
            getArgument<4, A...>(args[4])));
        else
          static_assert(sizeof(R) != sizeof(R));
      }
    }
};

#endif
