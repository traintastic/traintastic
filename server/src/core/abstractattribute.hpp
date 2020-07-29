/**
 * server/src/core/abstractattribute.hpp
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

#ifndef TRAINTASTIC_SERVER_CORE_ABSTRACTATTRIBUTE_HPP
#define TRAINTASTIC_SERVER_CORE_ABSTRACTATTRIBUTE_HPP

#include <string>
#include <traintastic/enum/attributename.hpp>
#include <traintastic/enum/valuetype.hpp>

class InterfaceItem;

class AbstractAttribute
{
  protected:
    InterfaceItem& m_item;
    const AttributeName m_name;
    const ValueType m_type;

    void changed();

  public:
    AbstractAttribute(InterfaceItem& item, AttributeName name, ValueType type);

    inline InterfaceItem& item() const { return m_item; }
    inline AttributeName name() const { return m_name; }
    inline ValueType type() const { return m_type; }

    virtual bool toBool() const = 0;
    virtual int64_t toInt64() const = 0;
    virtual double toDouble() const = 0;
    virtual std::string toString() const = 0;
};

#endif
