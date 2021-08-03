/**
 * server/src/log/log.cpp
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

#include "log.hpp"
#include "consolelogger.hpp"
#include "filelogger.hpp"
#include "memorylogger.hpp"

std::list<std::unique_ptr<Logger>> Log::s_loggers;

void Log::enableConsoleLogger()
{
  enable<ConsoleLogger>();
}

void Log::enableFileLogger(const std::filesystem::path& filename)
{
  enable<FileLogger>(filename);
}

void Log::disableFileLogger()
{
  disable<FileLogger>();
}

void Log::enableMemoryLogger(uint32_t size)
{
  enable<MemoryLogger>(size);
}

void Log::disableMemoryLogger()
{
  disable<MemoryLogger>();
}

MemoryLogger* Log::getMemoryLogger()
{
  return get<MemoryLogger>();
}

void Log::log(std::string objectId, LogMessage message)
{
  const auto time = std::chrono::system_clock::now();
  for(const auto& logger : s_loggers)
    logger->log(time, objectId, message);
}

void Log::logFormatted(std::string objectId, LogMessage message, std::vector<std::string> args)
{
  const auto time = std::chrono::system_clock::now();
  for(const auto& logger : s_loggers)
    logger->log(time, objectId, message, args);
}
