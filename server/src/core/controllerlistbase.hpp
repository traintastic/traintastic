/**
 * server/src/core/controllerlistbase.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_CORE_CONTROLLERLISTBASE_HPP
#define TRAINTASTIC_SERVER_CORE_CONTROLLERLISTBASE_HPP

#include "abstractobjectlist.hpp"
#include "property.hpp"

class ControllerListBaseTableModel;

class ControllerListBase : public AbstractObjectList
{
  friend class ControllerListBaseTableModel;

  CLASS_ID("list.controller")

  private:
    std::vector<ObjectPtr> m_items;
    std::unordered_map<Object*, boost::signals2::connection> m_propertyChanged;
    std::vector<ControllerListBaseTableModel*> m_models;

    void rowCountChanged();

  protected:
    std::vector<ObjectPtr> getItems() const final { return m_items; }
    void setItems(const std::vector<ObjectPtr>& items) final { m_items = items; }

    void add(ObjectPtr object);
    void remove(const ObjectPtr& object);

  public:
    Property<uint32_t> length;

    ControllerListBase(Object& _parent, const std::string& parentPropertyName);

    TableModelPtr getModel() final;

    ObjectPtr getObject(uint32_t index) final;
};

#endif
