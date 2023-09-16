/**
 * server/src/hardware/interface/marklincan/marklincanlocomotivelisttablemodel.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#include "marklincanlocomotivelisttablemodel.hpp"
#include "marklincanlocomotivelist.hpp"
#include "../../protocol/marklincan/locomotivelist.hpp"
#include "../../../utils/displayname.hpp"
#include "../../../utils/tohex.hpp"

constexpr uint32_t columnName = 0;
constexpr uint32_t columnProtocol = 1;
constexpr uint32_t columnAddress = 2;
constexpr uint32_t columnMFXUID = 3;

MarklinCANLocomotiveListTableModel::MarklinCANLocomotiveListTableModel(MarklinCANLocomotiveList& list)
  : m_list{list.shared_ptr<MarklinCANLocomotiveList>()}
{
  assert(m_list);
  m_list->m_models.push_back(this);

  setColumnHeaders({
    DisplayName::Object::name,
    std::string_view{"decoder:protocol"},
    DisplayName::Hardware::address,
    std::string_view{"decoder:mfx_uid"}
    });

  setRowCount(m_list->data() ? m_list->data()->size() : 0);
}

MarklinCANLocomotiveListTableModel::~MarklinCANLocomotiveListTableModel()
{
  auto it = std::find(m_list->m_models.begin(), m_list->m_models.end(), this);
  assert(it != m_list->m_models.end());
  m_list->m_models.erase(it);
}

std::string MarklinCANLocomotiveListTableModel::getText(uint32_t column, uint32_t row) const
{
  if(m_list->data() && row < m_list->data()->size())
  {
    const auto& locomotive = m_list->data()->operator[](row);

    switch(column)
    {
      case columnName:
        return locomotive.name;

      case columnProtocol:
        return
          std::string("$")
            .append(EnumName<DecoderProtocol>::value)
            .append(":")
            .append(EnumValues<DecoderProtocol>::value.at(locomotive.protocol))
            .append("$");

      case columnAddress:
        return std::to_string(locomotive.protocol == DecoderProtocol::MFX ? locomotive.sid : locomotive.address);

      case columnMFXUID:
        if(locomotive.protocol == DecoderProtocol::MFX)
          return toHex(locomotive.mfxUID);
        return {};
    }
  }
  return {};
}
