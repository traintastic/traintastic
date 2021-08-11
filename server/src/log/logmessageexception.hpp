/**
 * server/src/log/logmessageexception.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_LOG_LOGMESSAGEEXCEPTION_HPP
#define TRAINTASTIC_SERVER_LOG_LOGMESSAGEEXCEPTION_HPP

#include <stdexcept>
#include <traintastic/enum/logmessage.hpp>

class LogMessageException : public std::exception
{
  private:
    LogMessage m_message;
    std::string m_what;

  public:
    LogMessageException(LogMessage message)
      : m_message{message}
      , m_what{"message:"}
    {
      m_what += logMessageChar(message);
      m_what.append(std::to_string(logMessageNumber(message)));
    }

    const char* what() const noexcept override
    {
      return m_what.c_str();
    }

    inline LogMessage message() const { return m_message; }
};

#endif
