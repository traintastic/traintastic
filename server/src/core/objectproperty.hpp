/**
 * server/src/core/objectproperty.hpp
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

#ifndef TRAINTASTIC_SERVER_CORE_OBJECTPROPERTY_HPP
#define TRAINTASTIC_SERVER_CORE_OBJECTPROPERTY_HPP

#include "abstractobjectproperty.hpp"
#include <functional>

template<class T>
class ObjectProperty : public AbstractObjectProperty
{
  public:
    using OnChanged = std::function<void(const std::shared_ptr<T>& value)>;
    using OnSet = std::function<bool(const std::shared_ptr<T>& value)>;

  protected:
    std::shared_ptr<T> m_value;
    OnChanged m_onChanged;
    OnSet m_onSet;

  public:
    ObjectProperty(Object* object, std::string_view name, const std::shared_ptr<T>& value, PropertyFlags flags);
    ObjectProperty(Object* object, std::string_view name, std::nullptr_t, PropertyFlags flags);
    ObjectProperty(Object* object, std::string_view name, const std::shared_ptr<T>& value, PropertyFlags flags, OnChanged onChanged, OnSet onSet);
    ObjectProperty(Object* object, std::string_view name, std::nullptr_t, PropertyFlags flags, OnChanged onChanged, OnSet onSet);
    ObjectProperty(Object* object, std::string_view name, const std::shared_ptr<T>& value, PropertyFlags flags, OnSet onSet);
    ObjectProperty(Object* object, std::string_view name, std::nullptr_t, PropertyFlags flags, OnSet onSet);

    const std::shared_ptr<T>& value() const;
    void setValue(const std::shared_ptr<T>& value);

    void setValueInternal(std::nullptr_t);
    void setValueInternal(const std::shared_ptr<T>& value);

    /*inline*/ const T* operator ->() const;
    /*inline*/ T* operator ->();
    /*inline*/ const T& operator *() const;
    /*inline*/ T& operator *();

    /*inline*/ operator bool() const;
    ObjectProperty<T>& operator =(const std::shared_ptr<T>& value);

    ObjectPtr toObject() const final;

    void fromObject(const ObjectPtr& value) final;
    void loadObject(const ObjectPtr& value) final;
};

//#include "objectproperty.tpp"

#endif
