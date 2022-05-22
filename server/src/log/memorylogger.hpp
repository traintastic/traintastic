/**
 * server/src/log/memorylogger.hpp
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

#ifndef TRAINTASTIC_SERVER_LOG_MEMORYLOGGER_HPP
#define TRAINTASTIC_SERVER_LOG_MEMORYLOGGER_HPP

#include "logger.hpp"
#include <list>
#include <functional>
#include <boost/signals2/signal.hpp>

class MemoryLogger : public Logger
{
  public:
    struct Log
    {
      std::chrono::system_clock::time_point time;
      std::string objectId;
      LogMessage message;
      std::vector<std::string>* args;

      Log(std::chrono::system_clock::time_point _time, std::string _objectId, LogMessage _message, std::vector<std::string>* _args = nullptr)
        : time{std::move(_time)}
        , objectId{std::move(_objectId)}
        , message{_message}
        , args{_args}
      {
      }
    };

  private:
    std::vector<Log> m_logs;
    size_t m_sizeMax;

    void add(std::chrono::system_clock::time_point time, std::string objectId, LogMessage message, std::vector<std::string>* args);
    uint32_t cleanUp();

  public:
    boost::signals2::signal<void(const MemoryLogger&, uint32_t added, uint32_t removed)> changed;

    MemoryLogger(uint32_t sizeMax);

    inline const Log& operator[](uint32_t index) const { return m_logs[index]; }
    inline uint32_t size() const { return static_cast<uint32_t>(m_logs.size()); }

    void log(const std::chrono::system_clock::time_point& time, std::string_view objectId, LogMessage message) final;
    void log(const std::chrono::system_clock::time_point& time, std::string_view objectId, LogMessage message, const std::vector<std::string>& args) final;
};

#endif
