/**
 * client/src/network/tablemodel.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021,2023,2025 Reinder Feenstra
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
#include "connection.hpp"

TableModel::TableModel(std::shared_ptr<Connection> connection, Handle handle, const QString& classId, QObject* parent) :
  QAbstractTableModel(parent),
  m_connection{std::move(connection)},
  m_handle{handle},
  m_classId{classId},
  m_rowCount{0}
{
}

TableModel::~TableModel()
{
  m_connection->releaseTableModel(this);
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(role == Qt::DisplayRole)
  {
    if(orientation == Qt::Vertical)
      return QString::number(1 + section);
    else if(orientation == Qt::Horizontal)
      return m_columnHeaders[section];
  }

  return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant TableModel::data(const QModelIndex& index, int role) const
{
  if(role == Qt::DisplayRole)
    return m_texts.value(ColumnRow(index.column(), index.row()));

  return QVariant{};
}

QString TableModel::getRowObjectId(int row) const
{
  // TODO: rename to get row id and get it from the server
  if(m_classId == "world_list_table_model")
    return m_texts.value(ColumnRow(1, row));
  else
    return m_texts.value(ColumnRow(0, row));
}

QString TableModel::getValue(int column, int row) const
{
  return m_texts.value(ColumnRow(column, row));
}

void TableModel::setRegionAll(bool enable)
{
  m_regionAll = enable;
  if(m_regionAll)
  {
    updateRegionAll();
  }
}

void TableModel::setRegion(uint32_t columnMin, uint32_t columnMax, uint32_t rowMin, uint32_t rowMax)
{
  if(m_region.columnMin != columnMin ||
     m_region.columnMax != columnMax ||
     m_region.rowMin != rowMin ||
     m_region.rowMax != rowMax)
  {
    m_region.columnMin = columnMin;
    m_region.columnMax = columnMax;
    m_region.rowMin = rowMin;
    m_region.rowMax = rowMax;

    m_connection->setTableModelRegion(this, m_region.columnMin, m_region.columnMax, m_region.rowMin, m_region.rowMax);
  }
}

void TableModel::setColumnHeaders(const QVector<QString>& values)
{
  if(m_columnHeaders != values)
  {
    beginResetModel();
    m_columnHeaders = values;
    if(m_regionAll)
    {
      updateRegionAll();
    }
    endResetModel();
  }
}

void TableModel::setRowCount(int value)
{
  if(m_rowCount != value)
  {
    beginResetModel();
    m_rowCount = value;
    if(m_regionAll)
    {
      updateRegionAll();
    }
    endResetModel();
  }
}

void TableModel::updateRegionAll()
{
  if(columnCount() == 0 || rowCount() == 0)
  {
    setRegion(1, 0, 1, 0); // select nothing
  }
  else
  {
    setRegion(0, columnCount() - 1, 0, rowCount() - 1);
  }
}
