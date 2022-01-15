/**
 * server/src/core/event.hpp
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

#ifndef TRAINTASTIC_SERVER_CORE_EVENT_HPP
#define TRAINTASTIC_SERVER_CORE_EVENT_HPP

#include "abstractevent.hpp"

template<class... Args>
class Event : public AbstractEvent
{
  friend class Object;

  private:
    template<class T, class... Tn>
    inline void addArguments(Arguments& args, T value, Tn... others)
    {
      if constexpr(value_type_v<T> == ValueType::Enum || value_type_v<T> == ValueType::Set)
        args.emplace_back(static_cast<int64_t>(value));
      else
        args.emplace_back(value);

      if constexpr(sizeof...(Tn) > 0)
        addArguments(args, others...);
    }

    template<class T>
    static constexpr std::pair<ValueType, std::string_view> getArgumentInfo()
    {
      if constexpr(is_set_v<T>)
        return {ValueType::Set, set_name_v<T>};
      else if constexpr(std::is_enum_v<T>)
        return {ValueType::Enum, EnumName<T>::value};
      else
        return {value_type_v<T>, {}};
    }

    static constexpr std::array<ArgumentInfo, sizeof...(Args)> s_argumentInfo = {{getArgumentInfo<Args>()...}};

  protected:
    void fire(Args... args)
    {
      Arguments arguments;
      if constexpr(sizeof...(Args) > 0)
      {
        arguments.reserve(sizeof...(Args));
        addArguments<Args...>(arguments, args...);
      }
      AbstractEvent::fire(arguments);
    }

  public:
    Event(Object& object, std::string_view name, EventFlags flags) :
      AbstractEvent(object, name, flags)
    {
    }

    std::pair<const ArgumentInfo*, size_t> argumentInfo() const final
    {
      return {s_argumentInfo.data(), s_argumentInfo.size()};
    }
};

#endif
