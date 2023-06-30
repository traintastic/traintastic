/**
 * client/src/network/abstractvectorproperty.hpp
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

#ifndef TRAINTASTIC_CLIENT_NETWORK_ABSTRACTVECTORPROPERTY_HPP
#define TRAINTASTIC_CLIENT_NETWORK_ABSTRACTVECTORPROPERTY_HPP

#include "baseproperty.hpp"
#include <traintastic/set/set.hpp>

class AbstractVectorProperty : public BaseProperty
{
  Q_OBJECT

  protected:
    explicit AbstractVectorProperty(Object& object, const QString& name, ValueType type, PropertyFlags flags) :
      BaseProperty(object, name, type, flags)
    {
    }

  public:
    bool empty() const { return size() == 0; }
    virtual int size() const = 0;

    virtual bool getBool(int /*index*/) const { Q_ASSERT(false); return false; }
    virtual int getInt(int /*index*/) const { Q_ASSERT(false); return 0; }
    virtual int64_t getInt64(int /*index*/) const { Q_ASSERT(false); return 0; }
    virtual double getDouble(int /*index*/) const { Q_ASSERT(false); return 0; }
    virtual QString getString(int /*index*/) const { Q_ASSERT(false); return ""; }
    virtual QVariant getVariant(int /*index*/) const { Q_ASSERT(false); return QVariant(); }

    template<typename T>
    T getEnum(int index) const
    {
      static_assert(std::is_enum_v<T>);
      return static_cast<T>(getInt64(index));
    }

    template<typename T>
    T getSet(int index) const
    {
      static_assert(is_set_v<T>);
      return static_cast<T>(getInt64(index));
    }
};

#endif
