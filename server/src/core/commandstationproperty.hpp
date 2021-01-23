/**
 * server/src/core/commandstationproperty.hpp
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

#ifndef TRAINTASTIC_SERVER_CORE_COMMANDSTATIONPROPERTY_HPP
#define TRAINTASTIC_SERVER_CORE_COMMANDSTATIONPROPERTY_HPP

#include "abstractobjectproperty.hpp"
#include <functional>

class CommandStation;

//! workaround for ObjectProperty<CommandStation>
class CommandStationProperty : public AbstractObjectProperty
{
  public:
    using OnSet = std::function<bool(const std::shared_ptr<CommandStation>& value)>;

  protected:
    std::shared_ptr<CommandStation> m_value;
    OnSet m_onSet;

  public:
    CommandStationProperty(Object* object, const std::string& name, const std::shared_ptr<CommandStation>& value, PropertyFlags flags);
    CommandStationProperty(Object* object, const std::string& name, std::nullptr_t, PropertyFlags flags);
    CommandStationProperty(Object* object, const std::string& name, const std::shared_ptr<CommandStation>& value, PropertyFlags flags, OnSet onSet);
    CommandStationProperty(Object* object, const std::string& name, std::nullptr_t, PropertyFlags flags, OnSet onSet);

    const std::shared_ptr<CommandStation>& value() const;

    void setValue(const std::shared_ptr<CommandStation>& value);
    void setValueInternal(const std::shared_ptr<CommandStation>& value);

    const CommandStation* operator ->() const;
    CommandStation* operator ->();

    const CommandStation& operator *() const;
    CommandStation& operator *();

    operator bool();

    CommandStationProperty& operator =(const std::shared_ptr<CommandStation>& value);

    ObjectPtr toObject() const final;
    void fromObject(const ObjectPtr& value) final;

    void load(const ObjectPtr& value) final;
};

#endif
