/**
 * server/src/core/interfaceitem.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019 Reinder Feenstra
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
#include <enum/category.hpp>

class Object;

class InterfaceItem
{
  public:
    using Attributes = std::unordered_map<AttributeName, std::unique_ptr<AbstractAttribute>>;

  protected:
    Object& m_object;
    const std::string m_name;
    Attributes m_attributes;

    template<typename T>
    InterfaceItem& addAttribute(AttributeName name, const T& value)
    {
      m_attributes.emplace(name, std::make_unique<Attribute<T>>(*this, name, value));
      return *this;
    }

    template<typename T>
    void setAttribute(AttributeName name, const T& value)
    {
      static_cast<Attribute<T>*>(m_attributes[name].get())->setValue(value);
    }

  public:
    InterfaceItem(Object& object, const std::string& name) :
      m_object{object},
      m_name{name}
    {
    }

    virtual ~InterfaceItem()
    {
    }

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

    inline InterfaceItem& addAttributeCategory(Category value) { return addAttribute(AttributeName::Category, value); }
    inline InterfaceItem& addAttributeEnabled(bool value) { return addAttribute(AttributeName::Enabled, value); }
    inline InterfaceItem& addAttributeVisible(bool value) { return addAttribute(AttributeName::Visible, value); }
    inline InterfaceItem& addAttributeObjectEditor(bool value) { return addAttribute(AttributeName::ObjectEditor, value); }
    inline InterfaceItem& addAttributeSubObject(bool value) { return addAttribute(AttributeName::SubObject, value); }

    template<typename T, std::size_t N>
    inline InterfaceItem& addAttributeValues(const std::array<T, N>& values) { return addAttribute(AttributeName::Values, values); }

    inline void setAttributeEnabled(bool value) { setAttribute(AttributeName::Enabled, value); }
};

#endif
