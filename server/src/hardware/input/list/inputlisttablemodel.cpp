/**
 * server/src/hardware/input/list/inputlisttablemodel.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2025 Reinder Feenstra
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

#include "inputlisttablemodel.hpp"
#include "inputlist.hpp"
#include "../inputcontroller.hpp"
#include "../../../core/objectproperty.tpp"
#include "../../../utils/displayname.hpp"

bool InputListTableModel::isListedProperty(std::string_view name)
{
  return
    name == "interface" ||
    name == "channel" ||
    name == "address";
}

static std::string_view displayName(InputListColumn column)
{
  switch(column)
  {
    case InputListColumn::Interface:
      return DisplayName::Hardware::interface;

    case InputListColumn::Channel:
      return DisplayName::Hardware::channel;

    case InputListColumn::Address:
      return DisplayName::Hardware::address;
  }
  assert(false);
  return {};
}

InputListTableModel::InputListTableModel(InputList& list)
  : ObjectListTableModel<Input>(list)
{
  std::vector<std::string_view> labels;

  for(auto column : inputListColumnValues)
  {
    if(contains(list.columns, column))
    {
      labels.emplace_back(displayName(column));
      m_columns.emplace_back(column);
    }
  }

  setColumnHeaders(std::move(labels));
}

std::string InputListTableModel::getText(uint32_t column, uint32_t row) const
{
  if(row < rowCount())
  {
    const Input& input = getItem(row);

    assert(column < m_columns.size());
    switch(m_columns[column])
    {
      case InputListColumn::Interface:
        if(const auto& interface = std::dynamic_pointer_cast<Object>(input.interface.value()))
        {
          if(auto* property = interface->getProperty("name"); property && !property->toString().empty())
            return property->toString();

          return interface->getObjectId();
        }
        return "";

      case InputListColumn::Channel:
        if(input.interface->inputChannels().size() > 1)
        {
          if(const auto* it = EnumValues<InputChannel>::value.find(input.channel); it != EnumValues<InputChannel>::value.end()) /*[[likely]]*/
          {
            return std::string("$").append(EnumName<InputChannel>::value).append(":").append(it->second).append("$");
          }
        }
        break;

      case InputListColumn::Address:
        return std::to_string(input.address.value());
    }
    assert(false);
  }

  return "";
}

void InputListTableModel::propertyChanged(BaseProperty& property, uint32_t row)
{
  std::string_view name = property.name();

  if(name == "interface")
    changed(row, InputListColumn::Interface);
  else if(name == "channel")
    changed(row, InputListColumn::Channel);
  else if(name == "address")
    changed(row, InputListColumn::Address);
}

void InputListTableModel::changed(uint32_t row, InputListColumn column)
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
