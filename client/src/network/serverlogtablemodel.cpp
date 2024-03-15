/**
 * client/src/network/serverlogtablemodel.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2023 Reinder Feenstra
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

#include "serverlogtablemodel.hpp"
#include <QDateTime>
#include "../network/connection.hpp"
#include "../theme/theme.hpp"
#include <traintastic/locale/locale.hpp>
#include <traintastic/enum/logmessage.hpp>

ServerLogTableModel::ServerLogTableModel(std::shared_ptr<Connection> connection)
  : m_columnHeaders{Locale::tr("server_log:time"), Locale::tr("server_log:object"), Locale::tr("server_log:code"), Locale::tr("server_log:message")}
  , m_connection{std::move(connection)}
  , m_iconDebug{Theme::getIcon("log.debug")}
  , m_iconInfo{Theme::getIcon("log.info")}
  , m_iconNotice{Theme::getIcon("log.notice")}
  , m_iconWarning{Theme::getIcon("log.warning")}
  , m_iconError{Theme::getIcon("log.error")}
  , m_iconCritical{Theme::getIcon("log.critical")}
  , m_iconFatal{Theme::getIcon("log.fatal")}
{
  m_connection->serverLog(*this, true);
}

ServerLogTableModel::~ServerLogTableModel()
{
  m_connection->serverLog(*this, false);
}

int ServerLogTableModel::rowCount(const QModelIndex& parent) const
{
   Q_UNUSED(parent);
   return static_cast<int>(m_logs.size());
}

QVariant ServerLogTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(role == Qt::DisplayRole && orientation == Qt::Horizontal)
    return m_columnHeaders[section];

  return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant ServerLogTableModel::data(const QModelIndex& index, int role) const
{
  if(role == Qt::DisplayRole)
  {
    const Log& log = m_logs.at(index.row());
    switch(index.column())
    {
      case columnTime:
        return QDateTime::fromSecsSinceEpoch(log.time / 1'000'000).toString("yyyy-MM-dd hh:mm:ss.").append(QString::number(log.time % 1'000'000).rightJustified(6, '0'));

      case columnObject:
        return log.object;

      case columnCode:
        return logMessageCode(log.code);

      case columnMessage:
        return log.message;
    }
  }
  else if (role == Qt::DecorationRole && index.column() == columnCode)
  {
    switch(logMessageChar(m_logs.at(index.row()).code))
    {
      case 'D': return m_iconDebug;
      case 'I': return m_iconInfo;
      case 'N': return m_iconNotice;
      case 'W': return m_iconWarning;
      case 'E': return m_iconError;
      case 'C': return m_iconCritical;
      case 'F': return m_iconFatal;
    }
  }

  return QVariant{};
}

void ServerLogTableModel::processMessage(const Message& message)
{
  if(Q_UNLIKELY(message.command() != Message::Command::ServerLog))
    return;

  beginResetModel();

  const auto logsSize = m_logs.size();

  const int remove = message.read<int>();
  for(int i = 0; i < remove; i++)
    m_logs.removeFirst();

  const int added = message.read<int>();
  for(int i = 0; i < added; i++)
  {
    Log log;
    log.time = message.read<int64_t>();
    log.object = QString::fromLatin1(message.read<QByteArray>());
    log.code = message.read<LogMessage>();
    log.message = Locale::tr("message:" + logMessageCode(log.code));
    const int argc = message.read<uint8_t>();
    for(int j = 0; j < argc; j++)
      log.message = log.message.arg(QString::fromUtf8(message.read<QByteArray>()));
    m_logs.append(log);
  }

  endResetModel();

  if(logsSize != m_logs.size())
    emit rowCountChanged();
}
