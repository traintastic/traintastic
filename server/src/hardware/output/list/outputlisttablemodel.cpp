/**
 * server/src/hardware/output/list/outputlisttablemodel.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022,2024 Reinder Feenstra
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

#include "outputlisttablemodel.hpp"
#include "outputlist.hpp"
#include "../addressoutput.hpp"
#include "../outputcontroller.hpp"
#include "../../../core/objectproperty.tpp"
#include "../../../utils/displayname.hpp"

bool OutputListTableModel::isListedProperty(std::string_view name)
{
  return
    name == "interface" ||
    name == "channel" ||
    name == "address";
}

static std::string_view displayName(OutputListColumn column)
{
  switch(column)
  {
    case OutputListColumn::Interface:
      return DisplayName::Hardware::interface;

    case OutputListColumn::Channel:
      return DisplayName::Hardware::channel;

    case OutputListColumn::Address:
      return DisplayName::Hardware::address;
  }
  assert(false);
  return {};
}

OutputListTableModel::OutputListTableModel(OutputList& list) :
  ObjectListTableModel<Output>(list)
{
  std::vector<std::string_view> labels;

  for(auto column : outputListColumnValues)
  {
    if(contains(list.columns, column))
    {
      labels.emplace_back(displayName(column));
      m_columns.emplace_back(column);
    }
  }

  setColumnHeaders(std::move(labels));
}

std::string OutputListTableModel::getText(uint32_t column, uint32_t row) const
{
  if(row < rowCount())
  {
    const Output& output = getItem(row);

    assert(column < m_columns.size());
    switch(m_columns[column])
    {
      case OutputListColumn::Interface:
        if(const auto& interface = std::dynamic_pointer_cast<Object>(output.interface.value()))
        {
          if(auto* property = interface->getProperty("name"); property && !property->toString().empty())
            return property->toString();

          return interface->getObjectId();
        }
        return "";

      case OutputListColumn::Channel:
        if(const auto* it = EnumValues<OutputChannel>::value.find(output.channel); it != EnumValues<OutputChannel>::value.end()) /*[[likely]]*/
        {
          return std::string("$").append(EnumName<OutputChannel>::value).append(":").append(it->second).append("$");
        }
        break;

      case OutputListColumn::Address:
        if(auto* addressOutput = dynamic_cast<const AddressOutput*>(&output))
        {
          return std::to_string(addressOutput->address.value());
        }
        return {};
    }
    assert(false);
  }

  return "";
}

void OutputListTableModel::propertyChanged(BaseProperty& property, uint32_t row)
{
  std::string_view name = property.name();

  if(name == "interface")
    changed(row, OutputListColumn::Interface);
  else if(name == "channel")
    changed(row, OutputListColumn::Channel);
  else if(name == "address")
    changed(row, OutputListColumn::Address);
}

void OutputListTableModel::changed(uint32_t row, OutputListColumn column)
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
