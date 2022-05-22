/**
 * server/src/log/filelogger.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_LOG_FILELOGGER_HPP
#define TRAINTASTIC_SERVER_LOG_FILELOGGER_HPP

#include "logger.hpp"
#include <mutex>
#include <fstream>
#include <traintastic/utils/stdfilesystem.hpp>

class FileLogger : public Logger
{
  private:
    std::ofstream m_file;
    std::mutex m_fileMutex;

    void write(const std::chrono::system_clock::time_point& time, std::string_view objectId, LogMessage code, std::string_view message);

  public:
    FileLogger(const std::filesystem::path& filename);

    void log(const std::chrono::system_clock::time_point& time, std::string_view objectId, LogMessage message) final;
    void log(const std::chrono::system_clock::time_point& time, std::string_view objectId, LogMessage message, const std::vector<std::string>& args) final;
};

#endif
