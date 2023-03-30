/**
 * server/src/hardware/identification/list/identificationlisttablemodel.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#include "identificationlisttablemodel.hpp"
#include "identificationlist.hpp"
#include "../../../core/objectproperty.tpp"
#include "../../../utils/displayname.hpp"

bool IdentificationListTableModel::isListedProperty(std::string_view name)
{
  return
    name == "id" ||
    name == "name" ||
    name == "interface" ||
    name == "channel" ||
    name == "address";
}

static std::string_view displayName(IdentificationListColumn column)
{
  switch(column)
  {
    case IdentificationListColumn::Id:
      return DisplayName::Object::id;

    case IdentificationListColumn::Name:
      return DisplayName::Object::name;

    case IdentificationListColumn::Interface:
      return DisplayName::Hardware::interface;

    case IdentificationListColumn::Channel:
      return DisplayName::Hardware::channel;

    case IdentificationListColumn::Address:
      return DisplayName::Hardware::address;
  }
  assert(false);
  return {};
}

IdentificationListTableModel::IdentificationListTableModel(IdentificationList& list)
  : ObjectListTableModel<Identification>(list)
{
  std::vector<std::string_view> labels;

  for(auto column : identificationListColumnValues)
  {
    if(contains(list.columns, column))
    {
      labels.emplace_back(displayName(column));
      m_columns.emplace_back(column);
    }
  }

  setColumnHeaders(std::move(labels));
}

std::string IdentificationListTableModel::getText(uint32_t column, uint32_t row) const
{
  if(row < rowCount())
  {
    const Identification& identification = getItem(row);

    assert(column < m_columns.size());
    switch(m_columns[column])
    {
      case IdentificationListColumn::Id:
        return identification.id;

      case IdentificationListColumn::Name:
        return identification.name;

      case IdentificationListColumn::Interface:
        if(const auto& interface = std::dynamic_pointer_cast<Object>(identification.interface.value()))
        {
          if(auto* property = interface->getProperty("name"); property && !property->toString().empty())
            return property->toString();

          return interface->getObjectId();
        }
        return "";

      case IdentificationListColumn::Channel:
      {
        const uint32_t channel = identification.channel.value();
        if(channel == IdentificationController::defaultIdentificationChannel)
          return "";

        if(const auto* aliasKeys = identification.channel.tryGetValuesAttribute(AttributeName::AliasKeys))
        {
          if(const auto* aliasValues = identification.channel.tryGetValuesAttribute(AttributeName::AliasValues))
          {
            assert(aliasKeys->length() == aliasValues->length());
            for(uint32_t i = 0; i < aliasKeys->length(); i++)
              if(aliasKeys->getInt64(i) == channel)
                return aliasValues->getString(i);
          }
        }

        return std::to_string(channel);
      }
      case IdentificationListColumn::Address:
        return std::to_string(identification.address.value());
    }
    assert(false);
  }

  return "";
}

void IdentificationListTableModel::propertyChanged(BaseProperty& property, uint32_t row)
{
  std::string_view name = property.name();

  if(name == "id")
    changed(row, IdentificationListColumn::Id);
  else if(name == "name")
    changed(row, IdentificationListColumn::Name);
  else if(name == "interface")
    changed(row, IdentificationListColumn::Interface);
  else if(name == "channel")
    changed(row, IdentificationListColumn::Channel);
  else if(name == "address")
    changed(row, IdentificationListColumn::Address);
}

void IdentificationListTableModel::changed(uint32_t row, IdentificationListColumn column)
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
