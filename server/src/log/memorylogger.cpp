/**
 * server/src/log/memorylogger.cpp
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

#include "memorylogger.hpp"

MemoryLogger::MemoryLogger(uint32_t sizeMax)
  : m_sizeMax{sizeMax}
{
}

void MemoryLogger::log(const std::chrono::system_clock::time_point& time, std::string_view objectId, LogMessage message)
{
  m_logs.emplace_back(time, objectId, message);
  changed(*this, 1, cleanUp());
}

void MemoryLogger::log(const std::chrono::system_clock::time_point& time, std::string_view objectId, LogMessage message, const std::vector<std::string>& args)
{
  m_logs.emplace_back(time, objectId, message, new std::vector<std::string>(args));
  changed(*this, 1, cleanUp());
}

uint32_t MemoryLogger::cleanUp()
{
  const uint32_t remove = (size() > m_sizeMax) ? (size() - m_sizeMax) : 0;
  m_logs.erase(m_logs.begin(), m_logs.begin() + remove);
  return remove;
}
