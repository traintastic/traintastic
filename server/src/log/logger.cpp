/**
 * server/src/log/logger.cpp
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

#include "logger.hpp"
#include <cassert>
#include <traintastic/locale/locale.hpp>

std::string_view Logger::toString(LogMessage message)
{
  std::string key;
  key.resize(32);
  int n = snprintf(key.data(), key.size() ,"message:%c%04d", logMessageChar(message), logMessageNumber(message));
  if(n < 0)
    return {};
  else if(n < static_cast<int>(key.size()))
    key.resize(n);

  assert(Locale::tr(key) != key);
  return Locale::tr(key);
}

std::string Logger::toString(LogMessage message, const std::vector<std::string>& args)
{
  std::string s{toString(message)};

  for(size_t i = 0; i < args.size(); i++)
  {
    const std::string placeholder = std::string("%").append(std::to_string(1 + i));
    auto pos = s.find(placeholder);
    while(pos != std::string::npos)
    {
      if(pos + placeholder.size() == s.size() || s[pos + placeholder.size()] < '0' || s[pos + placeholder.size()] > '9')
      {
        s.replace(pos, placeholder.size(), args[i]);
        pos += args[i].size();
      }
      else
        pos += placeholder.size();

      pos = s.find(placeholder, pos);
    }
  }

  return s;
}
