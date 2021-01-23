/**
 * server/src/core/commandstationproperty.cpp
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

#include "commandstationproperty.hpp"
#include "../hardware/commandstation/commandstation.hpp"

using T = CommandStation;

CommandStationProperty::CommandStationProperty(Object* object, const std::string& name, const std::shared_ptr<T>& value, PropertyFlags flags) :
  AbstractObjectProperty(object, name, flags),
  m_value{value}
{
}

CommandStationProperty::CommandStationProperty(Object* object, const std::string& name, std::nullptr_t, PropertyFlags flags) :
  CommandStationProperty(object, name, std::shared_ptr<T>(), flags)
{
}

CommandStationProperty::CommandStationProperty(Object* object, const std::string& name, const std::shared_ptr<T>& value, PropertyFlags flags, OnSet onSet) :
  CommandStationProperty(object, name, value, flags)
{
  m_onSet = onSet;
}

CommandStationProperty::CommandStationProperty(Object* object, const std::string& name, std::nullptr_t, PropertyFlags flags, OnSet onSet) :
  CommandStationProperty(object, name, std::shared_ptr<T>(), flags, onSet)
{
}

const std::shared_ptr<T>& CommandStationProperty::value() const
{
  return m_value;
}

void CommandStationProperty::setValue(const std::shared_ptr<T>& value)
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
 }

void CommandStationProperty::setValueInternal(const std::shared_ptr<T>& value)
{
  if(m_value != value)
  {
    m_value = value;
    changed();
  }
}

const T* CommandStationProperty::operator ->() const
{
  return m_value.get();
}

T* CommandStationProperty::operator ->()
{
  return m_value.get();
}

const T& CommandStationProperty::operator *() const
{
  return *m_value;
}

T& CommandStationProperty::operator *()
{
  return *m_value;
}

CommandStationProperty::operator bool()
{
  return m_value.operator bool();
}

CommandStationProperty& CommandStationProperty::operator =(const std::shared_ptr<T>& value)
{
  setValue(value);
  return *this;
}

ObjectPtr CommandStationProperty::toObject() const
{
  return std::dynamic_pointer_cast<Object>(m_value);
}

void CommandStationProperty::fromObject(const ObjectPtr& value)
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

void CommandStationProperty::load(const ObjectPtr& value)
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
