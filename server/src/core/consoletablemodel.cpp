/**
 * server/src/core/consoletablemodel.cpp - Console table model
 *
 * This file is part of the traintastic source code
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

#include "consoletablemodel.hpp"
#include "console.hpp"
#include <chrono>
#include <iomanip>

constexpr uint32_t columnTime = 0;
constexpr uint32_t columnLevel = 1;
constexpr uint32_t columnId = 2;
constexpr uint32_t columnMessage = 3;

ConsoleTableModel::ConsoleTableModel(Console& console) :
  TableModel(),
  m_console{console}
{
  m_console.m_models.push_back(this);
  setColumnHeaders({"console:time", "console:level", "console:object", "console:message"});
  setRowCount(static_cast<uint32_t>(m_console.m_logs.size()));
}

ConsoleTableModel::~ConsoleTableModel()
{
  auto it = std::find(m_console.m_models.begin(), m_console.m_models.end(), this);
  assert(it != m_console.m_models.end());
  m_console.m_models.erase(it);
}

std::string ConsoleTableModel::getText(uint32_t column, uint32_t row) const
{
  if(row < rowCount())
  {
    const auto& log = m_console.m_logs[row];

    switch(column)
    {
      case columnTime:
      {
        const std::chrono::time_point<std::chrono::system_clock> time(std::chrono::seconds(log.time_s));
        const auto tm = std::chrono::system_clock::to_time_t(time);
        std::string s;
        s.resize(32);
        size_t n = strftime(s.data(), s.size(), "%F %T", std::localtime(&tm));
        n += snprintf(s.data() + n, s.size() - n, ".%06u", log.time_us);
        s.resize(n);
        return s;
      }
      case columnLevel:
        return to_string(log.level);

      case columnId:
        return log.id;

      case columnMessage:
        return log.message;

      default:
        assert(false);
        break;
    }
  }
  return "";
}

void ConsoleTableModel::logAdded()
{
  setRowCount(m_console.m_logs.size());
}
