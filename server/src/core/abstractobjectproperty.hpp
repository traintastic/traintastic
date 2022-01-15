/**
 * server/src/core/abstractobjectproperty.hpp
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

#ifndef TRAINTASTIC_SERVER_CORE_ABSTRACTOBJECTPROPERTY_HPP
#define TRAINTASTIC_SERVER_CORE_ABSTRACTOBJECTPROPERTY_HPP

#include "abstractproperty.hpp"

class AbstractObjectProperty : public AbstractProperty
{
  public:
    AbstractObjectProperty(Object* object, std::string_view name, PropertyFlags flags) :
      AbstractProperty(*object, name, ValueType::Object, flags)
    {
    }

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

    int64_t toInt64() const final
    {
      throw conversion_error();
    }

    double toDouble() const final
    {
      throw conversion_error();
    }

    std::string toString() const final
    {
      throw conversion_error();
    }

    nlohmann::json toJSON() const final
    {
      throw conversion_error();
    }

    void fromBool(bool /*value*/) final
    {
      throw conversion_error();
    }

    void fromInt64(int64_t) /*value*/ final
    {
      throw conversion_error();
    }

    void fromDouble(double /*value*/) final
    {
      throw conversion_error();
    }

    void fromString(const std::string& /*value*/) final
    {
      throw conversion_error();
    }

    void loadJSON(const nlohmann::json& /*value*/) final
    {
      throw conversion_error();
    }
};

#endif
