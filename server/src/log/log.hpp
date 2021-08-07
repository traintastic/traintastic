/**
 * server/src/log/log.hpp
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

#ifndef TRAINTASTIC_SERVER_LOG_LOG_HPP
#define TRAINTASTIC_SERVER_LOG_LOG_HPP

#include <string>
#include <vector>
#include <type_traits>
#include <boost/system/error_code.hpp>
#include <traintastic/enum/logmessage.hpp>
#include <traintastic/utils/stdfilesystem.hpp>
#include "../core/object.hpp"

class Logger;
class MemoryLogger;

class Log
{
  private:
    static std::list<std::unique_ptr<Logger>> s_loggers;

    Log() = default;

    template<class T, class... Ts>
    inline static void append(std::vector<std::string>& list, const T& value, const Ts&... others)
    {
      if constexpr(std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view> || std::is_same_v<T, const char*>)
        list.emplace_back(value);
      else if constexpr((std::is_integral_v<T> && !std::is_enum_v<T>) || std::is_floating_point_v<T>)
        list.emplace_back(std::to_string(value));
      else if constexpr(std::is_same_v<T, boost::system::error_code> || std::is_same_v<T, std::error_code>)
        list.emplace_back(value.message());
      else if constexpr(std::is_same_v<T, std::filesystem::path>)
        list.emplace_back(value.string());
      else if constexpr(std::is_base_of_v<Object, T>)
        list.emplace_back(value.getObjectId());
      else if constexpr(std::is_base_of_v<std::exception, T>)
        list.emplace_back(value.what());
      else
        list.emplace_back(toString(value));

      if constexpr(sizeof...(Ts) > 0)
        append(list, std::forward<const Ts&>(others)...);
    }

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

    inline static void log(const Object& object, LogMessage message)
    {
      log(object.getObjectId(), message);
    }

    template<class... Args>
    static void log(std::string objectId, LogMessage message, const Args&... args)
    {
      std::vector<std::string> list;
      list.reserve(sizeof...(Args));
      append(list, std::forward<const Args&>(args)...);
      logFormatted(std::move(objectId), message, std::move(list));
    }

    template<class... Args>
    inline static void log(const Object& object, LogMessage message, const Args&... args)
    {
      log(object.getObjectId(), message, std::forward<const Args&>(args)...);
    }
};

#endif
