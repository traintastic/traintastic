/**
 * server/src/core/property.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2022-2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_CORE_PROPERTY_HPP
#define TRAINTASTIC_SERVER_CORE_PROPERTY_HPP

#include "abstractproperty.hpp"
#include <traintastic/utils/valuetypetraits.hpp>
#include "to.hpp"
#include <functional>
#include <traintastic/enum/enum.hpp>

template<typename T>
class Property : public AbstractProperty
{
  public:
    using OnChanged = std::function<void(const T& value)>;
    using OnSet = std::function<bool(T& value)>;

  protected:
    T m_value;
    OnChanged m_onChanged;
    OnSet m_onSet;

    /*void loadValue(const nlohmann::json& value) final
    {
      m_value = to<T>(value);
    }*/

  public:
    Property(Object* object, std::string_view name, const T& value, PropertyFlags flags) :
      AbstractProperty(*object, name, value_type<T>::value, flags),
      m_value{value}
    {
      //static_assert(property_type<T>::value != PropertyType::Invalid);
    }

    Property(Object* object, std::string_view name, const T& value, PropertyFlags flags, OnChanged onChanged) :
      Property(object, name, value, flags)
    {
      m_onChanged = std::move(onChanged);
    }

    Property(Object* object, std::string_view name, const T& value, PropertyFlags flags, OnChanged onChanged, OnSet onSet) :
      Property(object, name, value, flags, onChanged)
    {
      m_onSet = std::move(onSet);
    }

    Property(Object* object, std::string_view name, const T& value, PropertyFlags flags, std::nullptr_t, OnSet onSet) :
      Property(object, name, value, flags)
    {
      m_onSet = std::move(onSet);
    }

    const T& value() const
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

      if constexpr(std::is_integral_v<T> || std::is_floating_point_v<T>)
      {
        if(auto it = m_attributes.find(AttributeName::Min); it != m_attributes.end())
        {
          const T min = static_cast<Attribute<T>&>(*it->second).value();

          if(value < min)
          {
            if constexpr(std::is_floating_point_v<T>)
            {
              if(value > min - std::numeric_limits<T>::epsilon() * 100)
                value = min;
              else
                throw out_of_range_error();
            }
            else
              throw out_of_range_error();
          }
        }

        if(auto it = m_attributes.find(AttributeName::Max); it != m_attributes.end())
        {
          const T max = static_cast<Attribute<T>&>(*it->second).value();

          if(value > max)
          {
            if constexpr(std::is_floating_point_v<T>)
            {
              if(value < max + std::numeric_limits<T>::epsilon() * 100)
                value = max;
              else
                throw out_of_range_error();
            }
            else
              throw out_of_range_error();
          }
        }
      }

      if constexpr(std::is_enum_v<T> && !is_set_v<T>)
      {
        if(auto it = m_attributes.find(AttributeName::Values); it != m_attributes.end())
        {
          if(auto* span = dynamic_cast<SpanAttribute<T>*>(it->second.get()))
          {
            const auto values = span->values();
            if(std::find(values.begin(), values.end(), value) == values.end())
              throw out_of_range_error();
          }
          else if(auto* vectorRef = dynamic_cast<VectorRefAttribute<T>*>(it->second.get()))
          {
            const auto* values = vectorRef->values();
            if(!values || std::find(values->begin(), values->end(), value) == values->end())
              throw out_of_range_error();
          }
          else if(auto* vector = dynamic_cast<VectorAttribute<T>*>(it->second.get()))
          {
            const auto& values = vector->values();
            if(std::find(values.begin(), values.end(), value) == values.end())
              throw out_of_range_error();
          }
        }
      }

      if(!m_onSet || m_onSet(value))
      {
        m_value = value;
        if(m_onChanged)
          m_onChanged(m_value);
        changed();
      }
      else
        throw invalid_value_error();
    }

    void setValueInternal(T value)
    {
      if(m_value != value)
      {
        m_value = value;
        changed();
      }
    }

    operator const T&() const
    {
      return m_value;
    }

    Property<T>& operator =(const T& value) // REMOVE !!!!!!
    {
      setValue(value);
      return *this;
    }

    std::string_view enumName() const final
    {
      if constexpr(std::is_enum_v<T> && !is_set_v<T>)
        return EnumName<T>::value;

      assert(false);
      return "";
    }

    std::string_view setName() const final
    {
      if constexpr(is_set_v<T>)
        return set_name_v<T>;

      assert(false);
      return "";
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

    ObjectPtr toObject() const final
    {
      throw conversion_error();
    }

    nlohmann::json toJSON() const final
    {
      return to<nlohmann::json>(m_value);
    }

    void fromBool(bool value) final
    {
      setValue(to<T>(value));
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

    void fromObject(const ObjectPtr& /*value*/) final
    {
      throw conversion_error();
    }

    void loadJSON(const nlohmann::json& value) final
    {
      m_value = to<T>(value);
    }

    void loadObject(const ObjectPtr& /*value*/) final
    {
      throw conversion_error();
    }

};

template<class T, std::enable_if_t<is_set_v<T>>* = nullptr>
inline bool contains(const Property<T>& property, const T& mask)
{
  return (property.value() & mask) == mask;
}

#endif
