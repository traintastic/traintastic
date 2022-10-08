/**
 * client/src/programming/lncv/lncvprogramminglistmodel.cpp
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

#include "lncvprogramminglistmodel.hpp"
#include "../../network/tablemodel.hpp"

LNCVProgrammingListModel::LNCVProgrammingListModel(TableModelPtr tableModel, QObject* parent)
  : QAbstractListModel(parent)
  , m_tableModel{std::move(tableModel)}
{
  m_tableModel->setRegion(0, std::numeric_limits<int>::max(), 0, std::numeric_limits<int>::max());
}

int LNCVProgrammingListModel::rowCount(const QModelIndex& parent) const
{
  return 1 + m_tableModel->rowCount(parent);
}

QVariant LNCVProgrammingListModel::data(const QModelIndex& index, int role) const
{
  if(index.row() == 0)
    return role == Qt::DisplayRole ? QVariant("") : QVariant{};

  return m_tableModel->data(QAbstractListModel::index(index.row() - 1, index.column()), role);
}
