/**
 * server/src/core/abstractvaluesattribute.hpp
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

#ifndef TRAINTASTIC_SERVER_CORE_ABSTRACTVALUESATTRIBUTE_HPP
#define TRAINTASTIC_SERVER_CORE_ABSTRACTVALUESATTRIBUTE_HPP

#include "abstractattribute.hpp"

class AbstractValuesAttribute : public AbstractAttribute
{
  public:
    AbstractValuesAttribute(InterfaceItem& _item, AttributeName _name, ValueType _type) :
      AbstractAttribute(_item, _name, _type)
    {
    }

    virtual uint32_t length() const = 0;

    virtual bool getBool(uint32_t index) const = 0;
    virtual int64_t getInt64(uint32_t index) const = 0;
    virtual double getDouble(uint32_t index) const = 0;
    virtual std::string getString(uint32_t index) const = 0;
};

#endif
