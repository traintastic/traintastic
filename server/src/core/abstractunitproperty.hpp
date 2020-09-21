/**
 * server/src/core/abstractunitproperty.hpp
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

#ifndef TRAINTASTIC_SERVER_CORE_ABSTRACTUNITPROPERTY_HPP
#define TRAINTASTIC_SERVER_CORE_ABSTRACTUNITPROPERTY_HPP

#include "abstractproperty.hpp"
#include "valuetypetraits.hpp"
#include "to.hpp"

class AbstractUnitProperty : public AbstractProperty
{
  public:
    AbstractUnitProperty(Object& object, const std::string& name, ValueType type, PropertyFlags flags) :
      AbstractProperty(object, name, type, flags)
    {
    }

    virtual std::string_view unitName() const = 0;
    virtual int64_t unitValue() const = 0;
    virtual void setUnitValue(int64_t value) = 0;

    std::string_view enumName() const final
    {
      assert(false);
      return "";
    }

    std::string_view setName() const final
    {
      assert(false);
      return "";
    }

    bool toBool() const final
    {
      throw conversion_error();
    }

    ObjectPtr toObject() const final
    {
      throw conversion_error();
    }

    void fromBool(bool value) final
    {
      throw conversion_error();
    }

    void fromObject(const ObjectPtr& value) final
    {
      throw conversion_error();
    }

    void load(const ObjectPtr& value) final
    {
      throw conversion_error();
    }
};

#endif
