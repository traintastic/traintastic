/**
 * server/src/core/abstractmethod.hpp
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

#ifndef TRAINTASTIC_SERVER_CORE_ABSTRACTMETHOD_HPP
#define TRAINTASTIC_SERVER_CORE_ABSTRACTMETHOD_HPP

#include "interfaceitem.hpp"
#include "methodflags.hpp"
#include <vector>
#include <variant>
#include <stdexcept>
#include "objectptr.hpp"

class AbstractMethod : public InterfaceItem
{
  private:
    const MethodFlags m_flags;

  public:
    class MethodCallError : public std::runtime_error
    {
      public:
        MethodCallError(const std::string& what) :
          std::runtime_error(what)
        {
        }

        MethodCallError(const char* what) :
          std::runtime_error(what)
        {
        }
    };

    class InvalidNumberOfArgumentsError : public MethodCallError
    {
      public:
        InvalidNumberOfArgumentsError() :
          MethodCallError("invalid number of arguments")
        {
        }
    };

    class ArgumentError : public MethodCallError
    {
      public:
        const size_t index;

        ArgumentError(size_t _index, const std::string& what) :
          MethodCallError(what),
          index{_index}
        {
        }

        ArgumentError(size_t _index, const char* what) :
          MethodCallError(what),
          index{_index}
        {
        }
    };

    class OutOfRangeArgumentError : public ArgumentError
    {
      public:
        OutOfRangeArgumentError(size_t _index) :
          ArgumentError(_index, "argument: out of range")
        {
        }
    };

    class InvalidObjectArgumentError : public ArgumentError
    {
      public:
        InvalidObjectArgumentError(size_t _index) :
          ArgumentError(_index, "argument: invalid object")
        {
        }
    };

    using Argument = std::variant<bool, int64_t, double, std::string, ObjectPtr>;
    using Result = std::variant<std::monostate, bool, int64_t, double, std::string, ObjectPtr>;

    AbstractMethod(Object& object, const std::string& name, MethodFlags m_flags = noMethodFlags);

    inline bool isScriptCallable() const { return m_flags == MethodFlags::ScriptCallable; }

    inline MethodFlags flags() const { return m_flags; }

    virtual std::size_t argumentCount() const = 0;
    virtual std::vector<ValueType> argumentTypes() const = 0;
    virtual ValueType resultType() const = 0;
    virtual Result call(const std::vector<Argument>& args) = 0;
};

#endif
