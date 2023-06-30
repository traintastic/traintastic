/**
 * server/src/log/log.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_LOG_LOG_HPP
#define TRAINTASTIC_SERVER_LOG_LOG_HPP

#include <string>
#include <vector>
#include <traintastic/enum/logmessage.hpp>
#include "appendarguments.hpp"
#ifdef ENABLE_LOG_DEBUG
  #include "joinarguments.hpp"
#endif

class Logger;
class MemoryLogger;

class Log
{
  private:
    static std::list<std::unique_ptr<Logger>> s_loggers;

    Log() = default;

    template<class T>
    inline static T* get()
    {
      for(const auto& logger : s_loggers)
        if(T* p = dynamic_cast<T*>(logger.get()))
          return p;
      return nullptr;
    }

    template<class T, class... Args>
    inline static void enable(Args... args)
    {
      disable<T>();
      s_loggers.emplace_back(new T(std::forward<Args>(args)...));
    }

    template<class T>
    inline static void disable()
    {
      auto it = std::find_if(s_loggers.begin(), s_loggers.end(), [](const auto& logger){ return dynamic_cast<T*>(logger.get()); });
      if(it != s_loggers.end())
        s_loggers.erase(it);
    }

    static void logFormatted(std::string objectId, LogMessage message, std::vector<std::string> args);

  public:
    static void enableConsoleLogger();

    static void enableFileLogger(const std::filesystem::path& filename);
    static void disableFileLogger();

    static void enableMemoryLogger(uint32_t size);
    static void disableMemoryLogger();
    static MemoryLogger* getMemoryLogger();

    static void log(std::string objectId, LogMessage message);

    inline static void log(std::string_view objectId, LogMessage message)
    {
      log(std::string{objectId}, message);
    }

    inline static void log(const Object& object, LogMessage message)
    {
      log(object.getObjectId(), message);
    }

    inline static void log(std::string objectId, LogMessage message, const std::vector<std::string>& args)
    {
      logFormatted(std::move(objectId), message, args);
    }

    inline static void log(std::string_view objectId, LogMessage message, const std::vector<std::string>& args)
    {
      logFormatted(std::string{objectId}, message, args);
    }

    inline static void log(const Object& object, LogMessage message, const std::vector<std::string>& args)
    {
      logFormatted(object.getObjectId(), message, args);
    }

    template<class... Args>
    static void log(std::string objectId, LogMessage message, const Args&... args)
    {
      std::vector<std::string> list;
      list.reserve(sizeof...(Args));
      appendArguments(list, std::forward<const Args&>(args)...);
      logFormatted(std::move(objectId), message, std::move(list));
    }

    template<class... Args>
    inline static void log(std::string_view objectId, LogMessage message, const Args&... args)
    {
      std::vector<std::string> list;
      list.reserve(sizeof...(Args));
      appendArguments(list, std::forward<const Args&>(args)...);
      logFormatted(std::string{objectId}, message, std::move(list));
    }

    template<class... Args>
    inline static void log(const Object& object, LogMessage message, const Args&... args)
    {
      log(object.getObjectId(), message, std::forward<const Args&>(args)...);
    }

#ifdef ENABLE_LOG_DEBUG
    template<class... Args>
    inline static void debug(const Args&... args)
    {
      std::string s;
      joinArguments(s, std::forward<const Args&>(args)...);
      log(std::string(), LogMessage::D0000_X, s);
    }
#endif
};

#ifdef ENABLE_LOG_DEBUG
  #define LOG_DEBUG(...) Log::debug(__VA_ARGS__);
#else
  #define LOG_DEBUG(...)
#endif

#endif
