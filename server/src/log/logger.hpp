/**
 * server/src/log/logger.hpp
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

#ifndef TRAINTASTIC_SERVER_LOG_LOGGER_HPP
#define TRAINTASTIC_SERVER_LOG_LOGGER_HPP

#include <vector>
#include <string>
#include <string_view>
#include <chrono>
#include <traintastic/enum/logmessage.hpp>

#if defined(__MINGW32__) || defined(__MINGW64__)
//NOTE: MinGW does not yet support all C++11 std::put_time specifiers
//See bug: https://sourceforge.net/p/mingw-w64/bugs/793
#define TRAINTASTIC_LOG_DATE_FORMAT "%Y-%m-%d"
#define TRAINTASTIC_LOG_TIME_FORMAT "%H:%M:%S"
#else
#define TRAINTASTIC_LOG_DATE_FORMAT "%F"
#define TRAINTASTIC_LOG_TIME_FORMAT "%T"
#endif

class Logger
{
  protected:
    static std::string_view toString(LogMessage message);
    static std::string toString(LogMessage message, const std::vector<std::string>& args);

    Logger() = default;

  public:
    virtual ~Logger() = default;

    virtual void log(const std::chrono::system_clock::time_point& time, std::string_view objectId, LogMessage message) = 0;
    virtual void log(const std::chrono::system_clock::time_point& time, std::string_view objectId, LogMessage message, const std::vector<std::string>& args) = 0;
};

#endif
