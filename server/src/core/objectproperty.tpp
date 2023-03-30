/**
 * server/src/core/objectproperty.tpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021,2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_CORE_OBJECTPROPERTY_TPP
#define TRAINTASTIC_SERVER_CORE_OBJECTPROPERTY_TPP

#include "objectproperty.hpp"
#include "to.hpp"

template<class T>
ObjectProperty<T>::ObjectProperty(Object* object, std::string_view name, const std::shared_ptr<T>& value, PropertyFlags flags) :
  AbstractObjectProperty(object, name, flags),
  m_value{value}
{
}

template<class T>
ObjectProperty<T>::ObjectProperty(Object* object, std::string_view name, std::nullptr_t, PropertyFlags flags) :
  ObjectProperty(object, name, std::shared_ptr<T>(), flags)
{
}

template<class T>
ObjectProperty<T>::ObjectProperty(Object* object, std::string_view name, const std::shared_ptr<T>& value, PropertyFlags flags, OnChanged onChanged, OnSet onSet) :
  ObjectProperty(object, name, value, flags)
{
  m_onChanged = onChanged;
  m_onSet = onSet;
}

template<class T>
ObjectProperty<T>::ObjectProperty(Object* object, std::string_view name, std::nullptr_t, PropertyFlags flags, OnChanged onChanged, OnSet onSet) :
  ObjectProperty(object, name, std::shared_ptr<T>(), flags, onChanged, onSet)
{
}

template<class T>
ObjectProperty<T>::ObjectProperty(Object* object, std::string_view name, const std::shared_ptr<T>& value, PropertyFlags flags, OnSet onSet) :
  ObjectProperty(object, name, value, flags)
{
  m_onSet = onSet;
}

template<class T>
ObjectProperty<T>::ObjectProperty(Object* object, std::string_view name, std::nullptr_t, PropertyFlags flags, OnSet onSet) :
  ObjectProperty(object, name, std::shared_ptr<T>(), flags, onSet)
{
}

template<class T>
const std::shared_ptr<T>& ObjectProperty<T>::value() const
{
  return m_value;
}

template<class T>
void ObjectProperty<T>::setValue(const std::shared_ptr<T>& value)
{
  assert(isWriteable());
  if(m_value == value)
    return;
  else if(!isWriteable())
    throw not_writable_error();
  else if(!m_onSet || m_onSet(value))
  {
    m_value = value;
    if(m_onChanged)
      m_onChanged(m_value);
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

template<class T>
void ObjectProperty<T>::setValueInternal(std::nullptr_t)
{
  if(m_value)
  {
    m_value.reset();
    changed();
  }
}

template<class T>
void ObjectProperty<T>::setValueInternal(const std::shared_ptr<T>& value)
{
  if(m_value != value)
  {
    m_value = value;
    changed();
  }
}

template<class T>
inline const T* ObjectProperty<T>::operator ->() const
{
  return m_value.get();
}

template<class T>
inline T* ObjectProperty<T>::operator ->()
{
  return m_value.get();
}

template<class T>
inline const T& ObjectProperty<T>::operator *() const
{
  return *m_value;
}

template<class T>
inline T& ObjectProperty<T>::operator *()
{
  return *m_value;
}

template<class T>
inline ObjectProperty<T>::operator bool() const
{
  return m_value.operator bool();
}

template<class T>
ObjectProperty<T>& ObjectProperty<T>::operator =(const std::shared_ptr<T>& value)
{
  setValue(value);
  return *this;
}

template<class T>
ObjectPtr ObjectProperty<T>::toObject() const
{
  return std::dynamic_pointer_cast<Object>(m_value);
}

template<class T>
void ObjectProperty<T>::fromObject(const ObjectPtr& value)
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

template<class T>
void ObjectProperty<T>::loadObject(const ObjectPtr& value)
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

#endif
