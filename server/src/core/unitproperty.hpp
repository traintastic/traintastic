/**
 * server/src/core/unitproperty.hpp
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

#ifndef TRAINTASTIC_SERVER_CORE_UNITPROPERTY_HPP
#define TRAINTASTIC_SERVER_CORE_UNITPROPERTY_HPP

#include "abstractunitproperty.hpp"
#include <functional>
#include "valuetypetraits.hpp"
#include "to.hpp"

template<typename T, typename Unit>
class UnitProperty : public AbstractUnitProperty
{
  static_assert(value_type<T>::value == ValueType::Integer || value_type<T>::value == ValueType::Float);

  public:
    using OnChanged = std::function<void(T value, Unit unit)>;

  protected:
    T m_value;
    Unit m_unit;
    OnChanged m_onChanged;

  public:
    UnitProperty(Object& object, const std::string& name, T value, Unit unit, PropertyFlags flags) :
      AbstractUnitProperty(object, name, value_type<T>::value, flags),
      m_value{value},
      m_unit{unit}
    {
    }

    UnitProperty(Object& object, const std::string& name, T value, Unit unit, PropertyFlags flags, OnChanged onChanged) :
      UnitProperty(object, name, value, unit, flags)
    {
      m_onChanged = std::move(onChanged);
    }

    inline T value() const
    {
      return m_value;
    }

    void setValue(T value)
    {
      assert(isWriteable());
      if(m_value == value)
        return;
      else if(!isWriteable())
        throw not_writable_error();
      else //if(!m_onSet || m_onSet(value))
      {
        m_value = value;
        if(m_onChanged)
          m_onChanged(m_value, m_unit);
        changed();
      }
      //else
      //  throw invalid_value_error();
    }

    void setValueInternal(T value)
    {
      if(m_value != value)
      {
        m_value = value;
        changed();
      }
    }

    void setValueInternal(T value, Unit unit)
    {
      if(m_value != value || m_unit != unit)
      {
        m_value = value;
        m_unit = unit;
        changed();
      }
    }

    inline Unit unit() const
    {
      return m_unit;
    }

    void setUnit(Unit value)
    {
      if(m_unit == value)
        return;

      m_value = convertUnit(m_value, m_unit, value);
      m_unit = value;
      if(m_onChanged)
        m_onChanged(m_value, m_unit);
      changed();
    }

    std::string_view unitName() const final
    {
      return EnumName<Unit>::value;
    }

    int64_t unitValue() const final
    {
      return to<int64_t>(m_unit);
    }

    void setUnitValue(int64_t value)
    {
      setUnit(to<Unit>(value));
    }

    inline T getValue(Unit _unit) const
    {
      return convertUnit(m_value, m_unit, _unit);
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

    nlohmann::json toJSON() const final
    {
      nlohmann::json v;
      v["value"] = to<nlohmann::json>(m_value);
      v["unit"] = to<nlohmann::json>(m_unit);
      return v;
    }

    void fromInt64(int64_t value) final
    {
      setValue(to<T>(value));
    }

    void fromDouble(double value) final
    {
      setValue(to<T>(value));
    }

    void fromString(const std::string& value) final
    {
      setValue(to<T>(value));
    }

    void load(const nlohmann::json& value) final
    {
      m_value = to<T>(value["value"]);
      from_json(value["unit"], m_unit);
    }
};

template<typename T, typename Unit>
std::string toString(const UnitProperty<T, Unit>& property)
{
  return std::to_string(property.value()).append(" $").append(EnumName<Unit>::value).append(":").append(EnumValues<Unit>::value.at(property.unit())).append("$");
}

#endif
