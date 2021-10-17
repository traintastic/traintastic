/**
 * server/src/core/controllerlistbase.cpp
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

#include "controllerlistbase.hpp"
#include "controllerlistbasetablemodel.hpp"

ControllerListBase::ControllerListBase(Object& _parent, const std::string& parentPropertyName)
  : AbstractObjectList(_parent, parentPropertyName)
  , length{this, "length", 0, PropertyFlags::ReadOnly}
{
}

TableModelPtr ControllerListBase::getModel()
{
  return std::make_shared<ControllerListBaseTableModel>(*this);
}

void ControllerListBase::add(ObjectPtr controller)
{
  assert(controller);
  m_propertyChanged.emplace(controller.get(), controller->propertyChanged.connect(
    [this](BaseProperty& property)
    {
      if(!m_models.empty() && ControllerListBaseTableModel::isListedProperty(property.name()))
      {
        ObjectPtr obj = property.object().shared_from_this();
        const uint32_t rows = static_cast<uint32_t>(m_items.size());
        for(uint32_t row = 0; row < rows; row++)
          if(m_items[row] == obj)
          {
            for(auto& model : m_models)
              model->propertyChanged(property, row);
            break;
          }
      }
    }));
  m_items.emplace_back(std::move(controller));
  rowCountChanged();
}

void ControllerListBase::remove(const ObjectPtr& controller)
{
  assert(controller);
  auto it = std::find(m_items.begin(), m_items.end(), controller);
  if(it != m_items.end())
  {
    m_propertyChanged.erase(controller.get());
    m_items.erase(it);
    rowCountChanged();
  }
}

void ControllerListBase::rowCountChanged()
{
  const auto size = m_items.size();
  length.setValueInternal(static_cast<uint32_t>(size));
  for(auto& model : m_models)
    model->setRowCount(static_cast<uint32_t>(size));
}

