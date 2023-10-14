/**
 * client/src/network/error.cpp
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

#include "error.hpp"
#include <QMessageBox>
#include <traintastic/network/message.hpp>
#include <traintastic/locale/locale.hpp>

Error::Error(const Message& msg)
{
  code = msg.read<LogMessage>();
  const auto count = msg.read<Message::Length>();
  for(Message::Length i = 0; i < count; ++i)
  {
    args.emplace_back(QString::fromUtf8(msg.read<QByteArray>()));
  }
}

QString Error::toString() const
{
  QString str = Locale::tr("message:" + logMessageCode(code));
  for(const auto& arg : args)
    str = str.arg(arg);
  return str;
}

void Error::show(QWidget* parent) const
{
  if(isDebugLogMessage(code) || isInfoLogMessage(code) || isNoticeLogMessage(code))
  {
    QMessageBox::information(parent, logMessageCode(code), toString());
  }
  else if(isWarningLogMessage(code))
  {
    QMessageBox::warning(parent, logMessageCode(code), toString());
  }
  else if(isErrorLogMessage(code) || isCriticalLogMessage(code) || isFatalLogMessage(code))
  {
    QMessageBox::critical(parent, logMessageCode(code), toString());
  }
  else /*[[unlikely]]*/
  {
    assert(false);
  }
}
