/**
 * server/src/log/consolelogger.cpp
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

#include "consolelogger.hpp"
#include <iostream>
#include <iomanip>
#include "../os/localtime.hpp"

void ConsoleLogger::log(const std::chrono::system_clock::time_point& time, std::string_view objectId, LogMessage message)
{
  write(time, objectId, message, toString(message));
}

void ConsoleLogger::log(const std::chrono::system_clock::time_point& time, std::string_view objectId, LogMessage message, const std::vector<std::string>& args)
{
  write(time, objectId, message, toString(message, args));
}

void ConsoleLogger::write(const std::chrono::system_clock::time_point& time, std::string_view objectId, LogMessage code, std::string_view message)
{
  const auto systemTime = std::chrono::system_clock::to_time_t(time);
  const auto us = std::chrono::duration_cast<std::chrono::microseconds>(time.time_since_epoch()) % 1000000;
  tm tm;

  std::lock_guard<std::mutex> lock(m_streamMutex);

  std::ostream& ss = (isErrorLogMessage(code) || isCriticalLogMessage(code) || isFatalLogMessage(code)) ? std::cerr : std::cout;
  ss
    << std::put_time(localTime(&systemTime, &tm), "%F %T") << '.' << std::setfill('0') << std::setw(6) << us.count() << ' '
    << objectId << ' '
    << logMessageChar(code) << std::setw(4) << logMessageNumber(code) << ": "
    << message
    << std::endl;
}
