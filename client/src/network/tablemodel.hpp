/**
 * client/src/network/tablemodel.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021,2023-2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_NETWORK_TABLEMODEL_HPP
#define TRAINTASTIC_CLIENT_NETWORK_TABLEMODEL_HPP

#include <QAbstractTableModel>
#include <memory>
#include "handle.hpp"

class Connection;

class TableModel final : public QAbstractTableModel
{
  Q_OBJECT

  friend class Connection;

  protected:
    using ColumnRow = std::pair<uint32_t, uint32_t>;

    std::shared_ptr<Connection> m_connection;
    Handle m_handle;
    const QString m_classId;
    QVector<QString> m_columnHeaders;
    int m_rowCount;
    struct Region
    {
      int rowMin = 0;
      int rowMax = -1;
      int columnMin = 0;
      int columnMax = -1;
    } m_region;
    QMap<ColumnRow, QString> m_texts;

    void setColumnHeaders(const QVector<QString>& values);
    void setRowCount(int value);

  public:
    explicit TableModel(std::shared_ptr<Connection> connection, Handle handle, const QString& classId, QObject* parent = nullptr);
    ~TableModel() final;

    Handle handle() const { return m_handle; }
    const QString& classId() const { return m_classId; }

    int columnCount(const QModelIndex& parent = QModelIndex()) const final { Q_UNUSED(parent); return m_columnHeaders.size(); }
    int rowCount(const QModelIndex& parent = QModelIndex()) const final { Q_UNUSED(parent); return m_rowCount; }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const final;
    QVariant data(const QModelIndex& index, int role) const final;

    QString getRowObjectId(int row) const;
    QString getValue(int column, int row) const;

    void setRegion(int columnMin, int columnMax, int rowMin, int rowMax);
};

#endif
