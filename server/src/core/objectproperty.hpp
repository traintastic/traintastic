/**
 * server/src/core/objectproperty.hpp
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

#ifndef SERVER_CORE_OBJECTPROPERTY_HPP
#define SERVER_CORE_OBJECTPROPERTY_HPP

#include "abstractproperty.hpp"
#include "to.hpp"
#include <functional>

template<class T>
class ObjectProperty : public AbstractProperty
{
  public:
    using OnSet = std::function<bool(const std::shared_ptr<T>& value)>;

  protected:
    std::shared_ptr<T> m_value;
    OnSet m_onSet;

  public:
    ObjectProperty(Object* object, const std::string& name, const std::shared_ptr<T>& value, PropertyFlags flags) :
      AbstractProperty(*object, name, ValueType::Object, flags),
      m_value{value}
    {
    }

    ObjectProperty(Object* object, const std::string& name, nullptr_t, PropertyFlags flags) :
      ObjectProperty(object, name, std::shared_ptr<T>(), flags)
    {
    }

    ObjectProperty(Object* object, const std::string& name, const std::shared_ptr<T>& value, PropertyFlags flags, OnSet onSet) :
      ObjectProperty(object, name, value, flags)
    {
      m_onSet = onSet;
    }

    ObjectProperty(Object* object, const std::string& name, nullptr_t, PropertyFlags flags, OnSet onSet) :
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

    inline operator bool()
    {
      return m_value.operator bool();
    }

    ObjectProperty<T>& operator =(const std::shared_ptr<T>& value)
    {
      setValue(value);
      return *this;
    }

    std::string enumName() const final
    {
      assert(false);
      return "";
    }

    bool toBool() const final
    {
      throw conversion_error();
    }

    int64_t toInt64() const final
    {
      throw conversion_error();
    }

    double toDouble() const final
    {
      throw conversion_error();
    }

    std::string toString() const final
    {
      throw conversion_error();
    }

    ObjectPtr toObject() const final
    {
      return std::dynamic_pointer_cast<Object>(m_value);
    }

    nlohmann::json toJSON() const final
    {
      throw conversion_error();
    }

    void fromBool(bool value) final
    {
      throw conversion_error();
    }

    void fromInt64(int64_t) final
    {
      throw conversion_error();
    }

    void fromDouble(double) final
    {
      throw conversion_error();
    }

    void fromString(const std::string&) final
    {
      throw conversion_error();
    }

    void fromObject(const ObjectPtr& value) final
    {
      if(value)
      {
        std::shared_ptr<T> v = std::dynamic_pointer_cast<T>(value);
        if(v)
          setValue(v);
        else
          throw conversion_error();
      }
      else
        setValue(nullptr);
    }

    void fromJSON(const nlohmann::json&) final
    {
      throw conversion_error();
    }
};

#endif
