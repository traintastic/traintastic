/**
 * server/src/core/spanattribute.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2023-2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_CORE_SPANATTRIBUTE_HPP
#define TRAINTASTIC_SERVER_CORE_SPANATTRIBUTE_HPP

#include <span>
#include "abstractvaluesattribute.hpp"
#include "to.hpp"

template<typename T, size_t N = std::dynamic_extent>
class SpanAttribute : public AbstractValuesAttribute
{
  public:
    typedef std::span<const T, N> Span;

  protected:
    Span m_values;

  public:
    SpanAttribute(InterfaceItem& _item, AttributeName _name, Span values) :
      AbstractValuesAttribute(_item, _name, value_type_v<T>),
      m_values{values}
    {
      static_assert(value_type_v<T> != ValueType::Invalid);
    }

    virtual uint32_t length() const final
    {
      return m_values.size();
    }

    virtual bool getBool(uint32_t index) const final
    {
      assert(index < length());
      return to<bool>(m_values[index]);
    }

    virtual int64_t getInt64(uint32_t index) const final
    {
      assert(index < length());
      return to<int64_t>(m_values[index]);
    }

    virtual double getDouble(uint32_t index) const final
    {
      assert(index < length());
      return to<double>(m_values[index]);
    }

    virtual std::string getString(uint32_t index) const final
    {
      assert(index < length());
      return to<std::string>(m_values[index]);
    }

    Span values() const
    {
      return m_values;
    }

    void setValues(Span values)
    {
      if(m_values.size() != values.size() || !std::equal(m_values.begin(), m_values.end(), values.begin()) || m_values.data() == values.data())
      {
        // if data() and size() are equal asume it has changed, we can't test it.
        m_values = values;
        changed();
      }
    }
};

#endif
