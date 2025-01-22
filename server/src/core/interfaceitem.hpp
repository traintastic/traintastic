/**
 * server/src/core/interfaceitem.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022 Reinder Feenstra
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
#include "spanattribute.hpp"
#include "vectorattribute.hpp"
#include "vectorrefattribute.hpp"

class Object;
class AbstractValuesAttribute;

class InterfaceItem
{
  friend struct Attributes;

  public:
    using Attributes = std::unordered_map<AttributeName, std::unique_ptr<AbstractAttribute>>;

  protected:
    Object& m_object;
    std::string_view m_name;
    Attributes m_attributes;

    template<typename T>
    void addAttribute(AttributeName name, const T& value)
    {
      assert(m_attributes.find(name) == m_attributes.end());
      m_attributes.emplace(name, std::make_unique<Attribute<T>>(*this, name, value));
    }

    template<typename T, size_t N>
    void addAttribute(AttributeName name, tcb::span<const T, N> values)
    {
      assert(m_attributes.find(name) == m_attributes.end());
      m_attributes.emplace(name, std::make_unique<SpanAttribute<T, N>>(*this, name, values));
    }

    template<typename T, size_t N>
    void addAttribute(AttributeName name, const std::array<T, N>& values)
    {
      addAttribute(name, tcb::span<const T, N>{values.data(), values.size()});
    }

    template<typename T>
    void addAttribute(AttributeName name, const std::vector<T>* values)
    {
      assert(m_attributes.find(name) == m_attributes.end());
      m_attributes.emplace(name, std::make_unique<VectorRefAttribute<T>>(*this, name, values));
    }

    template<typename T>
    void addAttribute(AttributeName name, std::vector<T> values)
    {
      assert(m_attributes.find(name) == m_attributes.end());
      m_attributes.emplace(name, std::make_unique<VectorAttribute<T>>(*this, name, std::move(values)));
    }

    template<typename T>
    void setAttribute(AttributeName name, const T& value)
    {
      assert(m_attributes.find(name) != m_attributes.end());
      static_cast<Attribute<T>*>(m_attributes[name].get())->setValue(value);
    }

    template<typename T, size_t N>
    void setAttribute(AttributeName name, tcb::span<const T, N> values)
    {
      assert(m_attributes.find(name) != m_attributes.end());
      static_cast<SpanAttribute<T, N>*>(m_attributes[name].get())->setValues(values);
    }

    template<typename T>
    void setAttribute(AttributeName name, std::vector<T> values)
    {
      assert(m_attributes.find(name) != m_attributes.end());
      static_cast<VectorAttribute<T>*>(m_attributes[name].get())->setValues(std::move(values));
    }

    template<typename T>
    void setAttribute(AttributeName name, const std::vector<T>* values)
    {
      assert(m_attributes.find(name) != m_attributes.end());
      static_cast<VectorRefAttribute<T>*>(m_attributes[name].get())->setValues(values);
    }

  public:
    InterfaceItem(const InterfaceItem&) = delete;
    InterfaceItem& operator =(const InterfaceItem&) = delete;

    InterfaceItem(Object& object, std::string_view name) :
      m_object{object},
      m_name{name}
    {
    }

    virtual ~InterfaceItem() = default;

    virtual bool isInternal() const = 0;

    Object& object() const
    {
      return m_object;
    }

    std::string_view name() const
    {
      return m_name;
    }

    const Attributes& attributes() const
    {
      return m_attributes;
    }

    template<typename T>
    T getAttribute(AttributeName name) const
    {
      assert(m_attributes.find(name) != m_attributes.end());
      return static_cast<const Attribute<T>*>(m_attributes.at(name).get())->value();
    }

    template<typename T>
    const SpanAttribute<T>& getSpanAttribute(AttributeName name) const
    {
      assert(m_attributes.find(name) != m_attributes.end());
      assert(dynamic_cast<const SpanAttribute<T>*>(m_attributes.at(name).get()));
      return *static_cast<const SpanAttribute<T>*>(m_attributes.at(name).get());
    }

    template<typename T>
    const std::vector<T>& getVectorAttribute(AttributeName name) const
    {
      assert(m_attributes.find(name) != m_attributes.end());
      assert(dynamic_cast<const VectorAttribute<T>*>(m_attributes.at(name).get()));
      return static_cast<const VectorAttribute<T>*>(m_attributes.at(name).get())->values();
    }

    const AbstractValuesAttribute* tryGetValuesAttribute(AttributeName name) const;
};

#endif
