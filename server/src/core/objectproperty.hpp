/**
 * server/src/core/objectproperty.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_CORE_OBJECTPROPERTY_HPP
#define TRAINTASTIC_SERVER_CORE_OBJECTPROPERTY_HPP

#include "abstractobjectproperty.hpp"
#include "to.hpp"
#include <functional>

template<class T>
class ObjectProperty : public AbstractObjectProperty
{
  public:
    using OnChanged = std::function<void(const std::shared_ptr<T>& value)>;
    using OnSet = std::function<bool(const std::shared_ptr<T>& value)>;

  protected:
    std::shared_ptr<T> m_value;
    OnChanged m_onChanged;
    OnSet m_onSet;

  public:
    ObjectProperty(Object* object, std::string_view name, const std::shared_ptr<T>& value, PropertyFlags flags) :
      AbstractObjectProperty(object, name, flags),
      m_value{value}
    {
    }

    ObjectProperty(Object* object, std::string_view name, std::nullptr_t, PropertyFlags flags) :
      ObjectProperty(object, name, std::shared_ptr<T>(), flags)
    {
    }

    ObjectProperty(Object* object, std::string_view name, const std::shared_ptr<T>& value, PropertyFlags flags, OnChanged onChanged, OnSet onSet) :
      ObjectProperty(object, name, value, flags)
    {
      m_onChanged = onChanged;
      m_onSet = onSet;
    }

    ObjectProperty(Object* object, std::string_view name, std::nullptr_t, PropertyFlags flags, OnChanged onChanged, OnSet onSet) :
      ObjectProperty(object, name, std::shared_ptr<T>(), flags, onChanged, onSet)
    {
    }

    ObjectProperty(Object* object, std::string_view name, const std::shared_ptr<T>& value, PropertyFlags flags, OnSet onSet) :
      ObjectProperty(object, name, value, flags)
    {
      m_onSet = onSet;
    }

    ObjectProperty(Object* object, std::string_view name, std::nullptr_t, PropertyFlags flags, OnSet onSet) :
      ObjectProperty(object, name, std::shared_ptr<T>(), flags, onSet)
    {
    }

    const std::shared_ptr<T>& value() const
    {
      return m_value;
    }

    void setValue(const std::shared_ptr<T>& value)
    {
      assert(isWriteable());
      if(m_value == value)
        return;
      else if(!isWriteable())
        throw not_writable_error();
      else if(!m_onSet || m_onSet(value))
      {
        m_value = value;
        changed();
      }
      else
        throw invalid_value_error();
      /*
      assert(isWriteable());
      if(isWriteable() && (!m_onSet || m_onSet(value)))
        setValueInternal(value);
        */
    }

    void setValueInternal(const std::shared_ptr<T>& value)
    {
      if(m_value != value)
      {
        m_value = value;
        changed();
      }
    }

    inline const T* operator ->() const
    {
      return m_value.get();
    }

    inline T* operator ->()
    {
      return m_value.get();
    }

    inline const T& operator *() const
    {
      return *m_value;
    }

    inline T& operator *()
    {
      return *m_value;
    }

    inline operator bool() const
    {
      return m_value.operator bool();
    }

    ObjectProperty<T>& operator =(const std::shared_ptr<T>& value)
    {
      setValue(value);
      return *this;
    }

    ObjectPtr toObject() const final
    {
      return std::dynamic_pointer_cast<Object>(m_value);
    }

    void fromObject(const ObjectPtr& value) final
    {
      if(value)
      {
        if(std::shared_ptr<T> v = std::dynamic_pointer_cast<T>(value))
          setValue(v);
        else
          throw conversion_error();
      }
      else
        setValue(nullptr);
    }

    void loadObject(const ObjectPtr& value) final
    {
      if(value)
      {
        if(std::shared_ptr<T> v = std::dynamic_pointer_cast<T>(value))
          m_value = v;
        else
          throw conversion_error();
      }
      else
        m_value.reset();
    }
};

#endif
