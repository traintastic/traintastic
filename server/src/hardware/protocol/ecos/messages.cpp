/**
 * server/src/hardware/protocol/ecos/messages.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022,2024 Reinder Feenstra
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

#include "messages.hpp"
#include <cassert>
#include <charconv>
#include "../../../utils/startswith.hpp"
#include "../../../utils/fromchars.hpp"
#include "../../../utils/rtrim.hpp"

namespace ECoS {

static const std::string_view startDelimiterReply = "<REPLY ";
static const std::string_view endDelimiter = "<END ";

bool parseRequest(std::string_view message, Request& request)
{
  // read command:
  size_t pos;
  if((pos = message.find('(')) == std::string_view::npos)
    return false;
  request.command = message.substr(0, pos);

  // read objectId:
  auto r = fromChars(message.substr(pos + 1), request.objectId);
  if(r.ec != std::errc())
    return false;

  // read arguments
  size_t n = r.ptr - message.data();
  while((pos = std::min(std::min(message.find(',', n), message.find(')', n)), message.find('[', n))) != std::string_view::npos)
  {
    if(message[pos] == '[')
    {
      if((pos = message.find(']', pos)) == std::string_view::npos)
        return false;
      if((pos = std::min(message.find(',', pos), message.find(')', pos))) == std::string_view::npos)
        return false;
    }
    while(message[n] == ' ' && n < pos)
      n++;
    if(pos > n)
      request.options.emplace_back(&message[n], pos - n);
    if(message[pos] == ')')
      break;
    n = pos + 1;
  }

  return true;
}

bool isReply(std::string_view message)
{
  return startsWith(message, startDelimiterReply);
}

bool parseReply(std::string_view message, Reply& reply)
{
  if(!isReply(message))
    return false;

  size_t n = startDelimiterReply.size();
  size_t pos;
  if((pos = message.find('(', n)) == std::string_view::npos)
    return false;

  // read command:
  reply.command = message.substr(n, pos - n);
  n = pos + 1;

  // read objectId
  auto r = std::from_chars(&message[n], &message[message.size() - 1], reply.objectId);
  if(r.ec != std::errc())
    return false;

  // read arguments
  n = r.ptr - message.data();
  while((pos = std::min(std::min(message.find(',', n), message.find(')', n)), message.find('[', n))) != std::string_view::npos)
  {
    if(message[pos] == '[')
    {
      if((pos = message.find(']', pos)) == std::string_view::npos)
        return false;
      if((pos = std::min(message.find(',', pos), message.find(')', pos))) == std::string_view::npos)
        return false;
    }
    while(message[n] == ' ' && n < pos)
      n++;
    if(pos > n)
      reply.options.emplace_back(&message[n], pos - n);
    if(message[pos] == ')')
      break;
    n = pos + 1;
  }
  if(pos == std::string_view::npos || pos + 1 >= message.size() || message[pos + 1] != '>')
    return false;

  // advance to next line
  n = pos + 2;
  while((message[n] == '\n' || message[n] == '\r') && n < message.size())
    n++;
  if(n >= message.size())
    return false;

  // find end
  size_t end;
  if((end = message.find(endDelimiter, n)) == std::string_view::npos)
    return false;

  // read lines
  if(end > n)
  {
    while((pos = message.find('\n', n)) < end)
    {
      reply.lines.emplace_back(rtrim({&message[n], pos - n}, '\r'));
      n = pos + 1;
    }
  }

  // read status code
  std::underlying_type_t<Status> status;
  r = std::from_chars(&message[end + endDelimiter.size()], &message[message.size() - 1], status);
  if(r.ec != std::errc())
    return false;
  reply.status = static_cast<Status>(status);
  n = r.ptr - message.data();

  // read status message
  while(message[n] != '(' && message[n] != '\n' && message[n] != '\r' && n < message.size())
    n++;
  if(n >= message.size() || message[n] != '(')
    return false;

  pos = ++n;
  while(message[pos] != ')' && message[pos] != '\n' && message[pos] != '\r' && n < message.size())
    pos++;
  if(pos >= message.size() || message[pos] != ')')
    return false;

  reply.statusMessage = message.substr(n, pos - n);

  return true;
}

bool parseEvent(std::string_view message, Event& event)
{
  static const std::string_view startDelimiter = "<EVENT ";

  if(!startsWith(message, startDelimiter))
    return false;

  size_t n = startDelimiter.size();
  size_t pos;

  // read objectId
  auto r = std::from_chars(&message[n], &message[message.size() - 1], event.objectId);
  if(r.ec != std::errc())
    return false;

  // advance to next line
  n = r.ptr - message.data() + 1;
  while((message[n] == '\n' || message[n] == '\r') && n < message.size())
    n++;
  if(n >= message.size())
    return false;

  // find end
  size_t end;
  if((end = message.find(endDelimiter, n)) == std::string_view::npos)
    return false;

  // read lines
  if(end > n)
  {
    while((pos = message.find('\n', n)) < end)
    {
      event.lines.emplace_back(rtrim({&message[n], pos - n}, '\r'));
      n = pos + 1;
    }
  }

  // read status code
  std::underlying_type_t<Status> status;
  r = std::from_chars(&message[end + endDelimiter.size()], &message[message.size() - 1], status);
  if(r.ec != std::errc())
    return false;
  event.status = static_cast<Status>(status);
  n = r.ptr - message.data();

  // read status message
  while(message[n] != '(' && message[n] != '\n' && message[n] != '\r' && n < message.size())
    n++;
  if(n >= message.size() || message[n] != '(')
    return false;

  pos = ++n;
  while(message[pos] != ')' && message[pos] != '\n' && message[pos] != '\r' && n < message.size())
    pos++;
  if(pos >= message.size() || message[pos] != ')')
    return false;

  event.statusMessage = message.substr(n, pos - n);

  return true;
}

bool parseId(std::string_view line, uint16_t& id)
{
  auto r = fromChars(line, id);
  return r.ec == std::errc();
}

bool parseLine(std::string_view text, Line& line)
{
  auto r = fromChars(text, line.objectId);
  if(r.ec != std::errc())
    return false;

  assert(line.values.empty());

  size_t n = r.ptr - text.data();
  while(n < text.size())
  {
    while(text[n] == ' ' && n < text.size())
      n++;

    if(n < text.size())
    {
      size_t pos = text.find('[', n);
      if(pos != std::string_view::npos)
      {
        std::string_view key{text.data() + n, pos - n};
        n = pos + 1;
        if(n >= text.size())
          return false;
        const bool quoted = (text[n] == '"');
        pos = quoted ? text.find("\"]", ++n) : text.find(']', n);
        if(pos == std::string_view::npos)
          return false;
        line.values.emplace(key, std::string_view{text.data() + n, pos - n});
        n = pos + (quoted ? 2 : 1);
      }
      else
        n = pos;
    }
  }

  return true;
}

bool parseOptionValue(std::string_view text, std::string_view& option, std::string_view& value)
{
  auto n = text.find('[');
  if(n == std::string_view::npos || *text.rbegin() != ']')
    return false;
  option = text.substr(0, n);
  value = text.substr(n + 1, text.size() - (n + 2));
  return true;
}

}
