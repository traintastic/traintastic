/**
 * server/src/hardware/interface/marklincan/marklincanlocomotivelist.cpp
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

#include "marklincanlocomotivelist.hpp"
#include "marklincanlocomotivelisttablemodel.hpp"
#include "../../protocol/marklincan/locomotivelist.hpp"

MarklinCANLocomotiveList::MarklinCANLocomotiveList(Object& parent_, std::string_view parentPropertyName)
  : SubObject(parent_, parentPropertyName)
{
}

void MarklinCANLocomotiveList::setData(std::shared_ptr<MarklinCAN::LocomotiveList> value)
{
  m_data = std::move(value);

  const uint32_t rowCount = m_data ? static_cast<uint32_t>(m_data->size()) : 0;

  for(auto& model : m_models)
  {
    model->setRowCount(rowCount);
  }
}

TableModelPtr MarklinCANLocomotiveList::getModel()
{
  return std::make_shared<MarklinCANLocomotiveListTableModel>(*this);
}
