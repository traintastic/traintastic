/**
 * server/src/core/vectorproperty.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023-2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_CORE_VECTORPROPERTY_HPP
#define TRAINTASTIC_SERVER_CORE_VECTORPROPERTY_HPP

#include "abstractvectorproperty.hpp"
#include <traintastic/utils/valuetypetraits.hpp>
#include "to.hpp"

template<typename T>
class VectorProperty : public AbstractVectorProperty
{
  public:
    using OnChanged = std::function<void(uint32_t, std::conditional_t<valueTypeByRef<T>(), const T&, T>  value)>;
    using OnSet = std::function<bool(uint32_t, T& value)>;

  private:
    std::vector<T> m_values;
    OnChanged m_onChanged;
    OnSet m_onSet;

  public:
    using const_iterator = typename std::vector<T>::const_iterator;

    VectorProperty(Object& object, std::string_view name, PropertyFlags flags)
      : AbstractVectorProperty(object, name, value_type<T>::value, flags)
    {
    }

    VectorProperty(Object& object, std::string_view name, std::initializer_list<T> values, PropertyFlags flags) :
      AbstractVectorProperty(object, name, value_type<T>::value, flags),
      m_values{values}
    {
    }

    VectorProperty(Object& object, std::string_view name, std::initializer_list<T> values, PropertyFlags flags, OnChanged onChanged)
      : VectorProperty(object, name, values, flags)
    {
       m_onChanged = std::move(onChanged);
    }

    VectorProperty(Object& object, std::string_view name, std::initializer_list<T> values, PropertyFlags flags, OnChanged onChanged, OnSet onSet)
      : VectorProperty(object, name, values, flags)
    {
       m_onChanged = std::move(onChanged);
       m_onSet = std::move(onSet);
    }

    VectorProperty(Object& object, std::string_view name, std::initializer_list<T> values, PropertyFlags flags, std::nullptr_t, OnSet onSet)
      : VectorProperty(object, name, values, flags)
    {
       m_onSet = std::move(onSet);
    }

    inline const_iterator begin() const { return m_values.begin(); }
    inline const_iterator end() const { return m_values.end(); }

    const T& operator [](size_t index) const
    {
      return m_values[index];
    }

    const std::vector<T>& operator *() const
    {
      return m_values;
    }

    void setValue(size_t index, T value)
    {
      assert(isWriteable());
      assert(index < size());
      if(m_values[index] == value)
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

      if(!m_onSet || m_onSet(index, value))
      {
        m_values[index] = value;
        if(m_onChanged)
          m_onChanged(index, m_values[index]);
        changed();
        return;
      }

      throw invalid_value_error();
    }

    void setValuesInternal(std::vector<T> values)
    {
      if(m_values != values)
      {
        m_values = std::move(values);
        changed();
      }
    }

    void setValueInternal(size_t index, T value)
    {
      assert(index < size());
      if(m_values[index] != value)
      {
        m_values[index] = value;
        changed();
      }
    }

    void appendInternal(T value)
    {
      m_values.emplace_back(std::move(value));
      changed();
    }

    void removeInternal(const T& value)
    {
      auto it = std::find(m_values.begin(), m_values.end(), value);
      if(it != m_values.end())
      {
        m_values.erase(it);
        changed();
      }
    }

    void eraseInternal(size_t index)
    {
      assert(index < size());
      m_values.erase(m_values.begin() + index);
      changed();
    }

    void clearInternal()
    {
      m_values.clear();
      changed();
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

    inline const T& front() const
    {
      return m_values.front();
    }

    inline T& front()
    {
      return m_values.front();
    }

    inline const T& back() const
    {
      return m_values.back();
    }

    inline T& back()
    {
      return m_values.back();
    }

    inline size_t size() const final
    {
      return m_values.size();
    }

    bool getBool(size_t index) const final
    {
      assert(index < size());
      return to<bool>(m_values[index]);
    }

    int64_t getInt64(size_t index) const final
    {
      return to<int64_t>(m_values[index]);
    }

    double getDouble(size_t index) const final
    {
      return to<double>(m_values[index]);
    }

    std::string getString(size_t index) const final
    {
      return to<std::string>(m_values[index]);
    }

    ObjectPtr getObject(size_t /*index*/) const final
    {
      throw conversion_error();
    }

    nlohmann::json toJSON() const final
    {
      nlohmann::json values = nlohmann::json::array();
      for(const auto& value : m_values)
        values.emplace_back(to<nlohmann::json>(value));
      return values;
    }

    void setBool(size_t index, bool value) final
    {
      setValue(index, to<T>(value));
    }

    void setInt64(size_t index, int64_t value) final
    {
      setValue(index, to<T>(value));
    }

    void setDouble(size_t index, double value) final
    {
      setValue(index, to<T>(value));
    }

    void setString(size_t index, const std::string& value) final
    {
      setValue(index, to<T>(value));
    }

    void setObject(size_t /*index*/, const ObjectPtr& /*value*/) final
    {
      throw conversion_error();
    }

    void loadJSON(const nlohmann::json& values) final
    {
      if(values.is_array())
      {
        m_values.resize(values.size());
        for(size_t i = 0; i < values.size(); i++)
          m_values[i] = to<T>(values[i]);
      }
      else
        throw conversion_error();
    }

    void loadObjects(std::span<ObjectPtr> /*values*/) final
    {
      throw conversion_error();
    }
};

#endif
