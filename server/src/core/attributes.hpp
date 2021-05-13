/**
 * server/src/core/attributes.hpp
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

#ifndef TRAINTASTIC_SERVER_CORE_ATTRIBUTES_HPP
#define TRAINTASTIC_SERVER_CORE_ATTRIBUTES_HPP

#include "interfaceitem.hpp"
#include "abstractobjectproperty.hpp"
#include "property.hpp"

struct Attributes
{
  static inline void addCategory(InterfaceItem& item, Category value)
  {
    item.addAttribute(AttributeName::Category, value);
  }

  template<size_t N>
  static inline void addClassList(InterfaceItem& item, const std::array<std::string_view, N>& classList)
  {
    item.addAttribute(AttributeName::ClassList, classList);
  }

  static inline void addDisplayName(InterfaceItem& item, std::string_view value)
  {
    item.addAttribute(AttributeName::DisplayName, value);
  }

  static inline void addEnabled(InterfaceItem& item, bool value)
  {
    item.addAttribute(AttributeName::Enabled, value);
  }

  static inline bool getEnabled(const InterfaceItem& item)
  {
    return item.getAttribute<bool>(AttributeName::Enabled);
  }

  static inline void setEnabled(InterfaceItem& item, bool value)
  {
    item.setAttribute(AttributeName::Enabled, value);
  }

  template<typename T>
  static inline void addMinMax(Property<T>& property, T min, T max)
  {
    property.addAttribute(AttributeName::Min, min);
    property.addAttribute(AttributeName::Max, max);
  }

  static inline void addVisible(InterfaceItem& item, bool value)
  {
    item.addAttribute(AttributeName::Visible, value);
  }

  static inline void setVisible(InterfaceItem& item, bool value)
  {
    item.setAttribute(AttributeName::Visible, value);
  }

  static inline void addObjectEditor(InterfaceItem& item, bool value)
  {
    item.addAttribute(AttributeName::ObjectEditor, value);
  }

  static inline void addObjectList(InterfaceItem& item, const AbstractObjectProperty& property)
  {
    std::string id(property.object().getObjectId());
    id.append(".");
    id.append(property.name());
    item.addAttribute(AttributeName::ObjectList, id);
  }

  template<typename T, size_t N>
  static inline void addValues(Property<T>& property, const std::array<T, N>& values)
  {
    property.addAttribute(AttributeName::Values, values);
  }


//inline InterfaceItem& addAttributeSubObject(bool value) { return addAttribute(AttributeName::SubObject, value); }
/*
template<typename T, std::size_t N>
inline InterfaceItem& addAttributeValues(const std::array<T, N>& values) { return addAttribute(AttributeName::Values, values); }
*/
};

#endif
