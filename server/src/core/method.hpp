/**
 * server/src/core/method.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2023,2025 Reinder Feenstra
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

template<class T>
class Method;

template<class R, class... A>
class Method<R(A...)> : public AbstractMethod
{
  private:
    template<std::size_t... I>
    Result call(const std::vector<Argument>& args, std::index_sequence<I...>);

  protected:
    std::function<R(A...)> m_function;

  public:
    Method(Object& object, std::string_view name, MethodFlags flags, std::function<R(A...)> function);

    Method(Object& object, std::string_view name, std::function<R(A...)> function);

    R operator()(A... args);

    std::span<const TypeInfo> argumentTypeInfo() const final;

    TypeInfo resultTypeInfo() const final;

    Result call(const std::vector<Argument>& args) final;
};

//#include "method.tpp"

#endif
