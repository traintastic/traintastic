/**
 * server/src/log/filelogger.cpp
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

#include "filelogger.hpp"
#include <iomanip>
#include <version.hpp>
#include "../os/localtime.hpp"

FileLogger::FileLogger(const std::filesystem::path& filename)
{
  // try create directory if it doesn't exist
  const auto path = filename.parent_path();
  if(!std::filesystem::is_directory(path))
    std::filesystem::create_directories(path);

  // open logfile and write marker
  m_file.open(filename, std::ios::app);
  m_file << "=== Traintastic v" TRAINTASTIC_VERSION_FULL << std::endl;
}

void FileLogger::log(const std::chrono::system_clock::time_point& time, std::string_view objectId, LogMessage message)
{
  write(time, objectId, message, toString(message));
}

void FileLogger::log(const std::chrono::system_clock::time_point& time, std::string_view objectId, LogMessage message, const std::vector<std::string>& args)
{
  write(time, objectId, message, toString(message, args));
}

void FileLogger::write(const std::chrono::system_clock::time_point& time, std::string_view objectId, LogMessage code, std::string_view message)
{
  const auto systemTime = std::chrono::system_clock::to_time_t(time);
  const auto us = std::chrono::duration_cast<std::chrono::microseconds>(time.time_since_epoch()) % 1000000;
  tm tm;

  std::lock_guard<std::mutex> lock(m_fileMutex);

  m_file
    << std::put_time(localTime(&systemTime, &tm), "%F;%T") << '.' << std::setfill('0') << std::setw(6) << us.count() << ';'
    << objectId << ';'
    << logMessageChar(code) << std::setw(4) << logMessageNumber(code) << ';'
    << message
    << std::endl;
}
