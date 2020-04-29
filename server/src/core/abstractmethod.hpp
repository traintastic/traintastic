/**
 * server/src/core/abstractmethod.hpp
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

#ifndef TRAINTASTIC_SERVER_CORE_ABSTRACTMETHOD_HPP
#define TRAINTASTIC_SERVER_CORE_ABSTRACTMETHOD_HPP

#include "interfaceitem.hpp"
#include <vector>
#include <variant>
#include "objectptr.hpp"

class AbstractMethod : public InterfaceItem
{
  public:
    using Argument = std::variant<bool, int64_t, double, std::string, ObjectPtr>;
    using Result = std::variant<std::monostate, bool, int64_t, double, std::string, ObjectPtr>;

    AbstractMethod(Object& object, const std::string& name);

    virtual std::size_t argumentCount() const = 0;
    virtual std::vector<ValueType> argumentTypes() const = 0;
    virtual ValueType resultType() const = 0;
    virtual Result call(const std::vector<Argument>& args) = 0;
};

#endif
