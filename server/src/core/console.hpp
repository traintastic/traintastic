/**
 * server/src/core/console.hpp
 *
 * This file is part of the traintastic source code.
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

#ifndef TRAINTASTIC_SERVER_CORE_CONSOLE_HPP
#define TRAINTASTIC_SERVER_CORE_CONSOLE_HPP

#include "object.hpp"
#include "property.hpp"
#include "table.hpp"
#include <memory>
#include <vector>

class ConsoleTableModel;

class Console : public Object, public Table
{
  friend class ConsoleTableModel;

  public:
    enum class Level
    {
      Debug = 0,
      Info = 1,
      Notice = 2,
      Warning = 3,
      Error = 4,
      Critical = 5,
      Fatal = 6,
    };

  protected:
    struct Log
    {
      int64_t time_s;
      uint32_t time_us;
      Level level;
      std::string id;
      std::string message;
    };

    std::vector<Log> m_logs;
    std::vector<ConsoleTableModel*> m_models;

  public:
    CLASS_ID("console");

    Property<uint32_t> count;

    Console();

    void log(Level level, std::string_view id, const std::string& message);
    void log(Level level, const std::string& id, const std::string& message);

    inline void debug(std::string_view id, const std::string& message) { log(Level::Debug, id, message); }
    inline void debug(const std::string& id, const std::string& message) { log(Level::Debug, id, message); }
    inline void info(std::string_view id, const std::string& message) { log(Level::Info, id, message); }
    inline void info(const std::string& id, const std::string& message) { log(Level::Info, id, message); }
    inline void notice(std::string_view id, const std::string& message) { log(Level::Notice, id, message); }
    inline void notice(const std::string& id, const std::string& message) { log(Level::Notice, id, message); }
    inline void warning(std::string_view id, const std::string& message) { log(Level::Warning, id, message); }
    inline void warning(const std::string& id, const std::string& message) { log(Level::Warning, id, message); }
    inline void error(std::string_view id, const std::string& message) { log(Level::Error, id, message); }
    inline void error(const std::string& id, const std::string& message) { log(Level::Error, id, message); }
    inline void critical(std::string_view id, const std::string& message) { log(Level::Critical, id, message); }
    inline void critical(const std::string& id, const std::string& message) { log(Level::Critical, id, message); }
    inline void fatal(std::string_view id, const std::string& message) { log(Level::Fatal, id, message); }
    inline void fatal(const std::string& id, const std::string& message) { log(Level::Fatal, id, message); }

    TableModelPtr getModel() final;
};

std::string to_string(const Console::Level& value);

#endif
