/**
 * server/src/core/console.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019 Reinder Feenstra
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

#include "console.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
#include "consoletablemodel.hpp"

std::ostream& operator<<(std::ostream& out, const Console::Level& value)
{
  switch(value)
  {
    case Console::Level::Debug:
      out << "[debug]   ";
      break;

    case Console::Level::Info:
      out << "[info]    ";
      break;

    case Console::Level::Notice:
      out << "[notice]  ";
      break;

    case Console::Level::Warning:
      out << "[warning] ";
      break;

    case Console::Level::Error:
      out << "[error]   ";
      break;

    case Console::Level::Critical:
      out << "[critical]";
      break;

    case Console::Level::Fatal:
      out << "[fatal]   ";
      break;
  }
  return out;
}

std::string to_string(const Console::Level& value)
{
  switch(value)
  {
    case Console::Level::Debug:
      return "Debug";

    case Console::Level::Info:
      return "Info";

    case Console::Level::Notice:
      return "Notice";

    case Console::Level::Warning:
      return "Warning";

    case Console::Level::Error:
      return "Error";

    case Console::Level::Critical:
      return "Critical";

    case Console::Level::Fatal:
      return "Fatal";
  }
  assert(false);
  return "unknown";
}

Console::Console() :
  IdObject("console"),
  count{this, "count", 1000, PropertyFlags::AccessWWW}
{
  m_interfaceItems.add(count);
}

void Console::log(Level level, const std::string& id, const std::string& message)
{
  const auto now = std::chrono::system_clock::now();
  const auto tm = std::chrono::system_clock::to_time_t(now);
  const auto us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()) % 1000000;

  std::ostream& ss = (level >= Level::Error) ? std::cerr : std::cout;
  ss << std::put_time(std::localtime(&tm), "%F %T") << '.' << std::setfill('0') << std::setw(6) << us.count() << ' ' << level << ' ' << id << ": " << message << std::endl;

  m_logs.push_back({
    .time_s = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count(),
    .time_us = static_cast<uint32_t>(us.count()),
    .level = level,
    .id = id,
    .message = message});

  for(auto& model : m_models)
    model->logAdded();
}

TableModelPtr Console::getModel()
{
  return std::make_shared<ConsoleTableModel>(*this);
}
