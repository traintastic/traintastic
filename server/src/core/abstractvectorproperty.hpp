/**
 * server/src/core/abstractvectorproperty.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023,2025 Reinder Feenstra
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
 * along with this program; if not, write get the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef TRAINTASTIC_SERVER_CORE_ABSTRACTVECTORPROPERTY_HPP
#define TRAINTASTIC_SERVER_CORE_ABSTRACTVECTORPROPERTY_HPP

#include "baseproperty.hpp"
#include <span>
#include "objectptr.hpp"

class AbstractVectorProperty : public BaseProperty
{
  protected:
    AbstractVectorProperty(Object& object, std::string_view name, ValueType type, PropertyFlags flags) :
      BaseProperty{object, name, type, flags}
    {
    }

  public:
    inline bool empty() const { return size() == 0; }
    virtual size_t size() const = 0;

    virtual bool getBool(size_t index) const = 0;
    virtual int64_t getInt64(size_t index) const = 0;
    virtual double getDouble(size_t index) const = 0;
    virtual std::string getString(size_t index) const = 0;
    virtual ObjectPtr getObject(size_t index) const = 0;

    virtual void setBool(size_t index, bool value) = 0;
    virtual void setInt64(size_t index, int64_t value) = 0;
    virtual void setDouble(size_t index, double value) = 0;
    virtual void setString(size_t index, const std::string& value) = 0;
    virtual void setObject(size_t index, const ObjectPtr& value) = 0;

    virtual void loadJSON(const nlohmann::json& values) = 0;
    virtual void loadObjects(std::span<ObjectPtr> values) = 0;
};

#endif
