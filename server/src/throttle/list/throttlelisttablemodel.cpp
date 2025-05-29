/**
 * server/src/throttle/list/throttlelisttablemodel.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022,2025 Reinder Feenstra
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

#include "throttlelisttablemodel.hpp"
#include "throttlelist.hpp"
#include "../webthrottle.hpp"
#include "../../core/objectproperty.tpp"
#include "../../train/train.hpp"
#include "../../utils/displayname.hpp"

bool ThrottleListTableModel::isListedProperty(std::string_view name)
{
  return
    name == "name" ||
    name == "train" ||
    name == "interface";
}

static std::string_view displayName(ThrottleListColumn column)
{
  switch(column)
  {
    case ThrottleListColumn::Name:
      return DisplayName::Object::name;

    case ThrottleListColumn::Train:
      return DisplayName::Vehicle::Rail::train;

    case ThrottleListColumn::Interface:
      return DisplayName::Hardware::interface;
  }
  assert(false);
  return {};
}

ThrottleListTableModel::ThrottleListTableModel(ThrottleList& list) :
  ObjectListTableModel<Throttle>(list)
{
  std::vector<std::string_view> labels;

  for(auto column : throttleListColumnValues)
  {
    if(contains(list.columns, column))
    {
      labels.emplace_back(displayName(column));
      m_columns.emplace_back(column);
    }
  }

  setColumnHeaders(std::move(labels));
}

std::string ThrottleListTableModel::getText(uint32_t column, uint32_t row) const
{
  if(row < rowCount())
  {
    const Throttle& throttle = getItem(row);

    assert(column < m_columns.size());
    switch(m_columns[column])
    {
      case ThrottleListColumn::Name:
        return throttle.name;

      case ThrottleListColumn::Train:
        if(throttle.train)
        {
          return throttle.train->name;
        }
        return {};

      case ThrottleListColumn::Interface:
        if(const auto* interfaceProperty = throttle.getObjectProperty("interface"); interfaceProperty)
        {
          if(const auto& interface = std::dynamic_pointer_cast<Object>(interfaceProperty->toObject()))
          {
            if(auto* property = interface->getProperty("name"); property && !property->toString().empty())
              return property->toString();

            return interface->getObjectId();
          }
        }
        else if(dynamic_cast<const WebThrottle*>(&throttle))
        {
          return "WebThrottle";
        }
        return "";
    }
    assert(false);
  }

  return "";
}

void ThrottleListTableModel::propertyChanged(BaseProperty& property, uint32_t row)
{
  std::string_view name = property.name();

  if(name == "name")
    changed(row, ThrottleListColumn::Name);
  else if(name == "train")
    changed(row, ThrottleListColumn::Train);
  else if(name == "interface")
    changed(row, ThrottleListColumn::Interface);
}

void ThrottleListTableModel::changed(uint32_t row, ThrottleListColumn column)
{
  for(size_t i = 0; i < m_columns.size(); i++)
  {
    if(m_columns[i] == column)
    {
      TableModel::changed(row, static_cast<uint32_t>(i));
      return;
    }
  }
}
