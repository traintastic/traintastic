/**
 * server/src/core/interfaceitem.hpp
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

#ifndef TRAINTASTIC_SERVER_CORE_INTERFACEITEM_HPP
#define TRAINTASTIC_SERVER_CORE_INTERFACEITEM_HPP

#include <unordered_map>
#include <string>
#include <memory>
#include "attribute.hpp"
#include "arrayattribute.hpp"
#include <traintastic/enum/category.hpp>

class Object;

class InterfaceItem
{
  friend struct Attributes;

  public:
    using Attributes = std::unordered_map<AttributeName, std::unique_ptr<AbstractAttribute>>;

  protected:
    Object& m_object;
    const std::string m_name;
    Attributes m_attributes;

    template<typename T>
    void addAttribute(AttributeName name, const T& value)
    {
      m_attributes.emplace(name, std::make_unique<Attribute<T>>(*this, name, value));
    }

    template<typename T, size_t N>
    void addAttribute(AttributeName name, const std::array<T, N>& values)
    {
      m_attributes.emplace(name, std::make_unique<ArrayAttribute<T, N>>(*this, name, values));
    }

    template<typename T>
    T getAttribute(AttributeName name) const
    {
      assert(m_attributes.find(name) != m_attributes.end());
      return static_cast<const Attribute<T>*>(m_attributes.at(name).get())->value();
    }

    template<typename T>
    void setAttribute(AttributeName name, const T& value)
    {
      assert(m_attributes.find(name) != m_attributes.end());
      static_cast<Attribute<T>*>(m_attributes[name].get())->setValue(value);
    }

  public:
    InterfaceItem(const InterfaceItem&) = delete;
    InterfaceItem& operator =(const InterfaceItem&) = delete;

    InterfaceItem(Object& object, const std::string& name) :
      m_object{object},
      m_name{name}
    {
    }

    virtual ~InterfaceItem() = default;

    Object& object() const
    {
      return m_object;
    }

    const std::string& name() const
    {
      return m_name;
    }

    const Attributes& attributes() const
    {
      return m_attributes;
    }

    inline void setAttributeEnabled(bool value) { setAttribute(AttributeName::Enabled, value); }
};

#endif
