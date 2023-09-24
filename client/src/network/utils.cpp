/**
 * client/src/network/utils.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020,2023 Reinder Feenstra
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

#include "utils.hpp"

QString errorCodeToText(Message::ErrorCode ec)
{
  using ErrorCode = Message::ErrorCode;

  Q_ASSERT(ec != ErrorCode::None);

  QString text;
  switch(ec)
  {
    case ErrorCode::InvalidCommand:
      text = "Invalid command";
      break;

    case ErrorCode::Failed:
      text = "Failed";
      break;

    case ErrorCode::AuthenticationFailed:
      text = "Authentication failed";
      break;

    case ErrorCode::InvalidSession:
      text = "Invalid session";
      break;

    case ErrorCode::UnknownObject:
      text = "Unknown object";
      break;

    case ErrorCode::ObjectNotTable:
      text = "Object not table";
      break;

    case ErrorCode::UnknownClassId:
      text = "Unknown class id";
      break;

    case ErrorCode::LogMessageException:
      assert(false);
      text = "LogMessageException";
      break;

    case ErrorCode::Other:
      assert(false);
      text = "Other";
      break;

    case ErrorCode::Unknown:
      text = "Unknown";
      break;

    case ErrorCode::None:
      break; // silence warning
  }
  return QString("Error %1: %2").arg(static_cast<int>(ec)).arg(text);
}
