/**
 * server/src/core/abstractobjectvectorproperty.hpp
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

#ifndef TRAINTASTIC_SERVER_CORE_ABSTRACTOBJECTVECTORPROPERTY_HPP
#define TRAINTASTIC_SERVER_CORE_ABSTRACTOBJECTVECTORPROPERTY_HPP

#include "abstractvectorproperty.hpp"

class AbstractObjectVectorProperty : public AbstractVectorProperty
{
  public:
    AbstractObjectVectorProperty(Object& object, std::string_view name, PropertyFlags flags) :
      AbstractVectorProperty(object, name, ValueType::Object, flags)
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

    nlohmann::json toJSON() const final
    {
      throw conversion_error();
    }

    bool getBool(size_t) const final
    {
      throw conversion_error();
    }

    int64_t getInt64(size_t) const final
    {
      throw conversion_error();
    }

    double getDouble(size_t) const final
    {
      throw conversion_error();
    }

    std::string getString(size_t) const final
    {
      throw conversion_error();
    }

    void setBool(size_t, bool) final
    {
      throw conversion_error();
    }

    void setInt64(size_t, int64_t) final
    {
      throw conversion_error();
    }

    void setDouble(size_t, double) final
    {
      throw conversion_error();
    }

    void setString(size_t, const std::string&) final
    {
      throw conversion_error();
    }

    void loadJSON(const nlohmann::json&) final
    {
      throw conversion_error();
    }
};

#endif
