/**
 * server/src/core/objectvectorproperty.hpp
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

#ifndef TRAINTASTIC_SERVER_CORE_OBJECTVECTORPROPERTY_HPP
#define TRAINTASTIC_SERVER_CORE_OBJECTVECTORPROPERTY_HPP

#include "abstractobjectvectorproperty.hpp"
#include "to.hpp"

template<class T>
class ObjectVectorProperty : public AbstractObjectVectorProperty
{
  private:
    std::vector<std::shared_ptr<T>> m_values;

    void setValue(size_t index, const std::shared_ptr<T>& value)
    {
      assert(index < size());
      m_values[index] = value;
    }

  public:
    using const_iterator = typename std::vector<std::shared_ptr<T>>::const_iterator;

    ObjectVectorProperty(Object& object, const std::string& name, std::initializer_list<std::shared_ptr<T>> values, PropertyFlags flags) :
      AbstractObjectVectorProperty(object, name, flags),
      m_values{values}
    {
    }

    inline const_iterator begin() const { return m_values.begin(); }
    inline const_iterator end() const { return m_values.end(); }

    const std::shared_ptr<T>& operator [](size_t index) const
    {
      return m_values[index];
    }

    const std::vector<std::shared_ptr<T>>& operator *() const
    {
      return m_values;
    }

    size_t indexOf(const T* value) const
    {
      const size_t sz = size();
      for(size_t i = 0; i < sz; i++)
        if(m_values[i].get() == value)
          return i;
      return sz;
    }

    inline size_t indexOf(const T& value) const
    {
      return indexOf(&value);
    }

    inline size_t indexOf(const std::shared_ptr<T>& value) const
    {
      return indexOf(value.get());
    }

    inline size_t size() const final
    {
      return m_values.size();
    }

    ObjectPtr getObject(size_t index) const final
    {
      assert(index < size());
      return std::dynamic_pointer_cast<Object>(m_values[index]);
    }

    void setObject(size_t index, const ObjectPtr& value) final
    {
      if(value)
      {
        if(std::shared_ptr<T> v = std::dynamic_pointer_cast<T>(value))
          setValue(index, v);
        else
          throw conversion_error();
      }
      else
        setValue(index, nullptr);
    }

    void appendInternal(std::shared_ptr<T> value)
    {
      m_values.emplace_back(std::move(value));
      changed();
    }

    void removeInternal(const std::shared_ptr<T>& value)
    {
      auto it = std::find(m_values.begin(), m_values.end(), value);
      if(it != m_values.end())
      {
        m_values.erase(it);
        changed();
      }
    }

    void load(std::vector<std::shared_ptr<T>> values)
    {
      m_values = std::move(values);
    }
};

#endif
