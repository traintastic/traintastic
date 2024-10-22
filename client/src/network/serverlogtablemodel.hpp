/**
 * client/src/network/serverlogtablemodel.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_CLIENT_NETWORK_SERVERLOGTABLEMODEL_HPP
#define TRAINTASTIC_CLIENT_NETWORK_SERVERLOGTABLEMODEL_HPP

#include <QAbstractTableModel>
#include <QIcon>
#include <memory>
#include <array>

class Connection;
class Message;
enum class LogMessage : uint32_t;

class ServerLogTableModel final : public QAbstractTableModel
{
  Q_OBJECT

  friend class Connection;

  private:
    struct Log
    {
      int64_t time; //!< us since unix epoch
      QString object;
      LogMessage code;
      QString message;
    };

    const std::array<QString, 4> m_columnHeaders;
    std::shared_ptr<Connection> m_connection;
    QList<Log> m_logs;
    int m_rowCount;
    QIcon m_iconDebug;
    QIcon m_iconInfo;
    QIcon m_iconNotice;
    QIcon m_iconWarning;
    QIcon m_iconError;
    QIcon m_iconCritical;
    QIcon m_iconFatal;

    void processMessage(const Message& message);

  public:
    static constexpr int columnTime = 0;
    static constexpr int columnObject = 1;
    static constexpr int columnCode = 2;
    static constexpr int columnMessage = 3;

    ServerLogTableModel(std::shared_ptr<Connection> connection);
    ~ServerLogTableModel();

    int columnCount(const QModelIndex& parent = QModelIndex()) const final { Q_UNUSED(parent); return static_cast<int>(m_columnHeaders.size()); }
    int rowCount(const QModelIndex& parent = QModelIndex()) const final;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const final;
    QVariant data(const QModelIndex& index, int role) const final;

  signals:
    void rowCountChanged();
};

#endif