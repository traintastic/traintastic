/**
 * server/src/core/vectorrefattribute.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_CORE_VECTORREFATTRIBUTE_HPP
#define TRAINTASTIC_SERVER_CORE_VECTORREFATTRIBUTE_HPP

#include <vector>
#include "abstractvaluesattribute.hpp"
#include "to.hpp"

template<typename T>
class VectorRefAttribute : public AbstractValuesAttribute
{
  private:
    const std::vector<T>* m_values;

  public:
    VectorRefAttribute(InterfaceItem& _item, AttributeName _name, const std::vector<T>* values) :
      AbstractValuesAttribute(_item, _name, value_type_v<T>),
      m_values{values}
    {
      static_assert(value_type_v<T> != ValueType::Invalid);
    }

    virtual uint32_t length() const final
    {
      return m_values ? m_values->size() : 0;
    }

    virtual bool getBool(uint32_t index) const final
    {
      assert(index < length());
      return to<bool>((*m_values)[index]);
    }

    virtual int64_t getInt64(uint32_t index) const final
    {
      assert(index < length());
      return to<int64_t>((*m_values)[index]);
    }

    virtual double getDouble(uint32_t index) const final
    {
      assert(index < length());
      return to<double>((*m_values)[index]);
    }

    virtual std::string getString(uint32_t index) const final
    {
      assert(index < length());
      return to<std::string>((*m_values)[index]);
    }

    const std::vector<T>* values() const
    {
      return m_values;
    }

    void setValues(const std::vector<T>* values)
    {
      if(m_values != values)
      {
        m_values = values;
        changed();
      }
    }

    void internalChanged()
    {
      changed();
    }
};

#endif
