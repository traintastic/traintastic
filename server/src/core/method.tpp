/**
 * server/src/core/method.tpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023,2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_CORE_METHOD_TPP
#define TRAINTASTIC_SERVER_CORE_METHOD_TPP

#include "method.hpp"
#include <limits>
#include <utility>
#include <traintastic/utils/valuetypetraits.hpp>
#include "../utils/is_shared_ptr.hpp"

template<std::size_t N, class... A>
using getArgumentType = typename std::tuple_element<N, std::tuple<A...>>::type;

template<std::size_t N, class A>
auto getArgument(const Argument& value)
{
  using T = std::remove_const_t<std::remove_reference_t<A>>;

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
inline AbstractMethod::Result toResult(const T& value)
{
  if constexpr(std::is_enum_v<T>)
    return static_cast<int64_t>(value);
  else
    return value;
}

template<class R, class... A> template<std::size_t... I>
AbstractMethod::Result Method<R(A...)>::call(const std::vector<Argument>& args, std::index_sequence<I...>)
{
  if(args.size() != sizeof...(A))
    throw InvalidNumberOfArgumentsError();

  if constexpr(std::is_same_v<R, void>)
  {
    m_function(getArgument<I, A>(args[I])...);
    return Result();
  }
  else
    return toResult(m_function(getArgument<I, A>(args[I])...));
}

template<class R, class... A>
class Method<R(A...)>;

template<class R, class... A>
Method<R(A...)>::Method(Object& object, std::string_view name, MethodFlags flags, std::function<R(A...)> function)
  : AbstractMethod(object, name, flags)
  , m_function{std::move(function)}
{
}

template<class R, class... A>
Method<R(A...)>::Method(Object& object, std::string_view name, std::function<R(A...)> function)
  : AbstractMethod(object, name)
  , m_function{std::move(function)}
{
}

template<class R, class... A>
R Method<R(A...)>::operator()(A... args)
{
  return m_function(args...);
}

template<class R, class... A>
std::span<const TypeInfo> Method<R(A...)>::argumentTypeInfo() const
{
  return {typeInfoArray<A...>};
}

template<class R, class... A>
TypeInfo Method<R(A...)>::resultTypeInfo() const
{
  return getTypeInfo<R>();
}

template<class R, class... A>
AbstractMethod::Result Method<R(A...)>::call(const std::vector<Argument>& args)
{
  return call(args, std::index_sequence_for<A...>{});
}

#endif
