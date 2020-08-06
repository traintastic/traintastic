/**
 * server/src/core/attributes.hpp
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

#ifndef TRAINTASTIC_SERVER_CORE_ATTRIBUTES_HPP
#define TRAINTASTIC_SERVER_CORE_ATTRIBUTES_HPP

#include "interfaceitem.hpp"

struct Attributes
{
  static inline void addCategory(InterfaceItem& item, Category value)
  {
    item.addAttribute(AttributeName::Category, value);
  }

  static inline void addEnabled(InterfaceItem& item, bool value)
  {
    item.addAttribute(AttributeName::Enabled, value);
  }

  static inline void addVisible(InterfaceItem& item, bool value)
  {
    item.addAttribute(AttributeName::Visible, value);
  }

  static inline void addObjectEditor(InterfaceItem& item, bool value)
  {
    item.addAttribute(AttributeName::ObjectEditor, value);
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
