/**
 * server/src/core/attributes.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2023 Reinder Feenstra
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
#include "method.hpp"
#include "object.hpp"
#include "property.hpp"
#include "unitproperty.hpp"
#include "vectorproperty.hpp"
#include <tcb/span.hpp>

struct Attributes
{
  template<class T>
  static inline void addAliases(Property<T>& property, const std::vector<T>* keys, const std::vector<std::string_view>* values)
  {
    assert((!keys && !values) || (keys && values && keys->size() == values->size()));
    property.addAttribute(AttributeName::AliasKeys, keys);
    property.addAttribute(AttributeName::AliasValues, values);
  }

  template<class R, class T>
  static inline void addAliases(Method<R(T)>& method, const std::vector<T>* keys, const std::vector<std::string_view>* values)
  {
    assert((!keys && !values) || (keys && values && keys->size() == values->size()));
    method.addAttribute(AttributeName::AliasKeys, keys);
    method.addAttribute(AttributeName::AliasValues, values);
  }

  template<class T>
  static inline void setAliases(Property<T>& property, const std::vector<T>* keys, const std::vector<std::string_view>* values)
  {
    assert((!keys && !values) || (keys && values && keys->size() == values->size()));
    property.setAttribute(AttributeName::AliasKeys, keys);
    property.setAttribute(AttributeName::AliasValues, values);
  }

  static inline void addCategory(InterfaceItem& item, std::string_view value)
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

  static inline void setEnabled(std::initializer_list<std::reference_wrapper<InterfaceItem>> items, bool value)
  {
    for(auto& item : items)
      item.get().setAttribute(AttributeName::Enabled, value);
  }

  template<typename T>
  static inline void addMinMax(Property<T>& property, T min, T max)
  {
    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>);
    property.addAttribute(AttributeName::Min, min);
    property.addAttribute(AttributeName::Max, max);
  }

  template<typename T>
  static inline void addMinMax(Property<T>& property, std::pair<T, T> range)
  {
    addMinMax(property, range.first, range.second);
  }

  template<class T, class Unit>
  static inline void addMinMax(UnitProperty<T, Unit>& property, T min, T max, Unit unit)
  {
    static_assert(std::is_floating_point_v<T>);
    property.addAttribute(AttributeName::Min, convertUnit(min, unit, property.unit()));
    property.addAttribute(AttributeName::Max, convertUnit(max, unit, property.unit()));
  }

  template<typename T>
  static inline void setMin(Property<T>& property, T value)
  {
    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>);
    property.setAttribute(AttributeName::Min, value);
  }

  template<class T, class Unit>
  static inline void setMin(UnitProperty<T, Unit>& property, T value, Unit unit)
  {
    static_assert(std::is_floating_point_v<T>);
    property.setAttribute(AttributeName::Min, convertUnit(value, unit, property.unit()));
  }

  template<typename T>
  static inline void setMax(Property<T>& property, T value)
  {
    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>);
    property.setAttribute(AttributeName::Max, value);
  }

  template<class T, class Unit>
  static inline void setMax(UnitProperty<T, Unit>& property, T value, Unit unit)
  {
    static_assert(std::is_floating_point_v<T>);
    property.setAttribute(AttributeName::Max, convertUnit(value, unit, property.unit()));
  }

  template<typename T>
  static inline void setMinMax(Property<T>& property, T min, T max)
  {
    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>);
    property.setAttribute(AttributeName::Min, min);
    property.setAttribute(AttributeName::Max, max);
  }

  template<typename T>
  static inline void setMinMax(Property<T>& property, std::pair<T, T> range)
  {
    setMinMax(property, range.first, range.second);
  }

  template<class T, class Unit>
  static inline void setMinMax(UnitProperty<T, Unit>& property, T min, T max, Unit unit)
  {
    static_assert(std::is_floating_point_v<T>);
    property.setAttribute(AttributeName::Min, convertUnit(min, unit, property.unit()));
    property.setAttribute(AttributeName::Max, convertUnit(max, unit, property.unit()));
  }

  static inline void addVisible(InterfaceItem& item, bool value)
  {
    item.addAttribute(AttributeName::Visible, value);
  }

  static inline void setVisible(InterfaceItem& item, bool value)
  {
    item.setAttribute(AttributeName::Visible, value);
  }

  static inline void setVisible(std::initializer_list<std::reference_wrapper<InterfaceItem>> items, bool value)
  {
    for(auto& item : items)
      item.get().setAttribute(AttributeName::Visible, value);
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

  template<class R, class T, size_t N>
  static inline void addValues(Method<R(T)>& method, const std::array<T, N>& values)
  {
    method.addAttribute(AttributeName::Values, values);
  }

  template<typename T>
  static inline void addValues(Property<T>& property, tcb::span<const T> values)
  {
    property.addAttribute(AttributeName::Values, values);
  }

  template<typename T, size_t N>
  static inline void addValues(Property<T>& property, const std::array<T, N>& values)
  {
    property.addAttribute(AttributeName::Values, values);
  }

  template<class R, class T>
  static inline void addValues(Method<R(T)>& method, const std::vector<T>* values)
  {
    method.addAttribute(AttributeName::Values, values);
  }

  template<typename T>
  static inline void addValues(Property<T>& property, const std::vector<T>* values)
  {
    property.addAttribute(AttributeName::Values, values);
  }

  template<typename T>
  static inline void addValues(Property<T>& property, std::vector<T> values)
  {
    property.addAttribute(AttributeName::Values, std::move(values));
  }

  template<class T, size_t N>
  static inline void addValues(VectorProperty<T>& property, const std::array<T, N>& values)
  {
    property.addAttribute(AttributeName::Values, values);
  }

  template<class R, class T>
  static inline void addValues(Method<R(T)>& method, std::vector<T> values)
  {
    method.addAttribute(AttributeName::Values, std::move(values));
  }

  template<typename T>
  static inline void setValues(Property<T>& property, tcb::span<const T> values)
  {
    property.setAttribute(AttributeName::Values, values);
  }

  template<typename T>
  static inline void setValues(Property<T>& property, const std::vector<T>* values)
  {
    property.setAttribute(AttributeName::Values, values);
  }

  template<typename T>
  static inline void setValues(Property<T>& property, std::vector<T> values)
  {
    property.setAttribute(AttributeName::Values, std::move(values));
  }

  template<class R, class T>
  static inline void setValues(Method<R(T)>& method, std::vector<T> values)
  {
    method.setAttribute(AttributeName::Values, std::move(values));
  }
};

#endif
