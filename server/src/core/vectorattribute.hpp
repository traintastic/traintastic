/**
 * server/src/core/vectorattribute.hpp
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

#ifndef TRAINTASTIC_SERVER_CORE_VECTORATTRIBUTE_HPP
#define TRAINTASTIC_SERVER_CORE_VECTORATTRIBUTE_HPP

#include "abstractattribute.hpp"
#include "to.hpp"
#include "valuetypetraits.hpp"

template<typename T>
class VectorAttribute : public AbstractAttribute
{
  protected:
    std::vector<T> m_values;

  public:
    VectorAttribute(InterfaceItem& item, AttributeName name, const T& values) :
      AbstractAttribute{item, name, value_type_v<T>},
      m_value{values}
    {
    }

    bool toBool() const final
    {
      return to<bool>(m_value);
    }

    int64_t toInt64() const final
    {
      return to<int64_t>(m_value);
    }

    double toDouble() const final
    {
      return to<double>(m_value);
    }

    std::string toString() const final
    {
      return to<std::string>(m_value);
    }

    void setValue(const T& value)
    {
      if(m_value != value)
      {
        m_value = value;
        changed();
      }
    }
};

#endif
