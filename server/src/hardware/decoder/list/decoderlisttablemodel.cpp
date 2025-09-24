/**
 * server/src/hardware/decoder/list/decoderlisttablemodel.cpp
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

#include "decoderlisttablemodel.hpp"
#include "decoderlist.hpp"
#include "../../../core/objectproperty.tpp"
#include "../../../vehicle/rail/railvehicle.hpp"
#include "../../../utils/displayname.hpp"

static std::string_view displayName(DecoderListColumn column)
{
  switch(column)
  {
    case DecoderListColumn::Id:
      return DisplayName::Object::id;

    case DecoderListColumn::Name:
      return DisplayName::Object::name;

    case DecoderListColumn::Interface:
      return DisplayName::Hardware::interface;

    case DecoderListColumn::Protocol:
      return "decoder:protocol";

    case DecoderListColumn::Address:
      return DisplayName::Hardware::address;
  }
  assert(false);
  return {};
}

bool DecoderListTableModel::isListedProperty(std::string_view name)
{
  return
    name == "id" ||
    name == "name" ||
    name == "interface" ||
    name == "protocol" ||
    name == "address";
}

DecoderListTableModel::DecoderListTableModel(DecoderList& list) :
  ObjectListTableModel<Decoder>(list)
{
  std::vector<std::string_view> labels;

  for(auto column : decoderListColumnValues)
  {
    if(contains(list.columns, column))
    {
      labels.emplace_back(displayName(column));
      m_columns.emplace_back(column);
    }
  }

  setColumnHeaders(std::move(labels));
}

std::string DecoderListTableModel::getText(uint32_t column, uint32_t row) const
{
  if(row < rowCount())
  {
    const Decoder& decoder = getItem(row);

    assert(column < m_columns.size());
    switch(m_columns[column])
    {
      case DecoderListColumn::Id:
        return decoder.id;

      case DecoderListColumn::Name:
        if(decoder.vehicle) [[likely]]
        {
          return decoder.vehicle->name;
        }
        return {};

      case DecoderListColumn::Interface:
        if(const auto& interface = std::dynamic_pointer_cast<Object>(decoder.interface.value()))
        {
          if(auto* property = interface->getProperty("name"); property && !property->toString().empty())
            return property->toString();

          return interface->getObjectId();
        }
        return "";

      case DecoderListColumn::Protocol:
        if(const auto* it = EnumValues<DecoderProtocol>::value.find(decoder.protocol); it != EnumValues<DecoderProtocol>::value.end())
          return std::string("$").append(EnumName<DecoderProtocol>::value).append(":").append(it->second).append("$");
        break;

      case DecoderListColumn::Address:
        if(hasAddress(decoder.protocol.value()))
          return decoder.address.toString();
        else
          return {};

      default:
        assert(false);
        break;
    }
  }

  return "";
}

void DecoderListTableModel::propertyChanged(BaseProperty& property, uint32_t row)
{
  std::string_view name = property.name();

  if(name == "id")
    changed(row, DecoderListColumn::Id);
  else if(name == "name")
    changed(row, DecoderListColumn::Name);
  else if(name == "interface")
    changed(row, DecoderListColumn::Interface);
  else if(name == "protocol")
    changed(row, DecoderListColumn::Protocol);
  else if(name == "address")
    changed(row, DecoderListColumn::Address);
}

void DecoderListTableModel::changed(uint32_t row, DecoderListColumn column)
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
