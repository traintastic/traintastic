/**
 * server/src/core/tablemodel.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2023 Reinder Feenstra
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

#include "tablemodel.hpp"

TableModel::TableModel() :
  m_rowCount{0}
{
}

void TableModel::setRegion(const Region& value)
{
  if(m_region != value)
  {
    if(!(value <= m_region))
    {
      Region update = value;

      if(update.columnMin == m_region.columnMin && update.columnMax == m_region.columnMax)
      {
        if(update.rowMin < m_region.rowMin && update.rowMax <= m_region.rowMax) // extend/move top
          update.rowMax = m_region.rowMin - 1;
        else if(update.rowMin >= m_region.rowMin && update.rowMax > m_region.rowMax) // extend/move bottom
          update.rowMin = m_region.rowMax + 1;
      }

      m_region = value;

      if(updateRegion)
        updateRegion(shared_ptr<TableModel>(), update);
    }
    else
      m_region = value;
  }
}

void TableModel::rowsChanged(uint32_t first, uint32_t last)
{
  Region update = m_region;
  if(update.rowMin <= last || update.rowMax >= first)
  {
    update.rowMin = std::max(update.rowMin, first);
    update.rowMax = std::min(update.rowMax, last);
    updateRegion(shared_ptr<TableModel>(), update);
  }
}

void TableModel::rowRemovedHack(uint32_t row)
{
  //Hack, tell clients to refresh from row onwards
  Region update = m_region;
  if(update.rowMin <= row && update.rowMax >= row)
  {
    update.rowMin = row;
    updateRegion(shared_ptr<TableModel>(), update);
  }
}

void TableModel::setColumnHeaders(std::vector<std::string_view> values)
{
  m_columnHeaders = std::move(values);
  if(columnHeadersChanged)
    columnHeadersChanged(shared_ptr<TableModel>());
}

void TableModel::setRowCount(uint32_t value)
{
  if(m_rowCount != value)
  {
    m_rowCount = value;
    if(rowCountChanged)
      rowCountChanged(shared_ptr<TableModel>());
  }
}

void TableModel::changed(uint32_t row, uint32_t column)
{
  if(updateRegion)
  {
    Region region{column, column, row, row};
    updateRegion(shared_ptr<TableModel>(), region);
  }
}
