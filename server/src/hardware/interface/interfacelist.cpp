/**
 * server/src/hardware/interface/interfacelist.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023-2024 Reinder Feenstra
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

#include "interfacelist.hpp"
#include "interfacelisttablemodel.hpp"
#include "interfaces.hpp"
#include "../decoder/list/decoderlist.hpp"
#include "../output/list/outputlist.hpp"
#include "../input/list/inputlist.hpp"
#include "../../world/world.hpp"
#include "../../world/getworld.hpp"
#include "../../core/attributes.hpp"
#include "../../core/method.tpp"
#include "../../core/objectproperty.tpp"
#include "../../utils/displayname.hpp"

InterfaceList::InterfaceList(Object& _parent, std::string_view parentPropertyName) :
  ObjectList<Interface>(_parent, parentPropertyName),
  create{*this, "create",
    [this](std::string_view interfaceClassId)
    {
      return Interfaces::create(getWorld(parent()), interfaceClassId);
    }},
  delete_{*this, "delete", std::bind(&InterfaceList::deleteMethodHandler, this, std::placeholders::_1)}
{
  const bool editable = contains(getWorld(parent()).state.value(), WorldState::Edit);

  Attributes::addDisplayName(create, DisplayName::List::create);
  Attributes::addEnabled(create, editable);
  Attributes::addClassList(create, Interfaces::classList());
  m_interfaceItems.add(create);

  Attributes::addDisplayName(delete_, DisplayName::List::delete_);
  Attributes::addEnabled(delete_, editable);
  m_interfaceItems.add(delete_);
}

InterfaceList::~InterfaceList()
{
  for(auto& it : m_statusPropertyChanged)
    it.second.disconnect();
}

TableModelPtr InterfaceList::getModel()
{
  return std::make_shared<InterfaceListTableModel>(*this);
}

void InterfaceList::worldEvent(WorldState state, WorldEvent event)
{
  ObjectList<Interface>::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(create, editable);
  Attributes::setEnabled(delete_, editable);
}

bool InterfaceList::isListedProperty(std::string_view name)
{
  return InterfaceListTableModel::isListedProperty(name);
}

void InterfaceList::objectAdded(const std::shared_ptr<Interface>& object)
{
  m_statusPropertyChanged.emplace(object.get(), object->status->propertyChanged.connect(std::bind(&InterfaceList::statusPropertyChanged, this, std::placeholders::_1)));
}

void InterfaceList::objectRemoved(const std::shared_ptr<Interface>& object)
{
  m_statusPropertyChanged[object.get()].disconnect();
  m_statusPropertyChanged.erase(object.get());
}

void InterfaceList::statusPropertyChanged(BaseProperty& property)
{
  if(!m_models.empty() && property.name() == "state")
  {
    ObjectPtr obj = static_cast<SubObject&>(property.object()).parent().shared_from_this();
    const uint32_t rows = static_cast<uint32_t>(m_items.size());
    for(uint32_t row = 0; row < rows; row++)
      if(m_items[row] == obj)
      {
        for(auto& model : m_models)
          static_cast<InterfaceListTableModel*>(model)->changed(row, InterfaceListTableModel::columnStatus);
        break;
      }
  }
}
