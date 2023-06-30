/**
 * server/src/core/objectvectorproperty.tpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_CORE_OBJECTVECTORPROPERTY_TPP
#define TRAINTASTIC_SERVER_CORE_OBJECTVECTORPROPERTY_TPP

#include "objectvectorproperty.hpp"

template<class T>
ObjectPtr ObjectVectorProperty<T>::getObject(size_t index) const
{
  assert(index < size());
  return std::static_pointer_cast<Object>(m_values[index]);
}

template<class T>
void ObjectVectorProperty<T>::setObject(size_t index, const ObjectPtr& value)
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

template<class T>
void ObjectVectorProperty<T>::loadObjects(tcb::span<ObjectPtr> values)
{
  std::vector<std::shared_ptr<T>> objects;
  objects.reserve(values.size());
  for(const auto& object : values)
    if(auto v = std::dynamic_pointer_cast<T>(object))
      objects.emplace_back(v);
    else
      throw conversion_error();
  m_values = std::move(objects);
}

#endif
