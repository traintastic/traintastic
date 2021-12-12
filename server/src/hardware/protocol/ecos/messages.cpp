#include "messages.hpp"
#include <cassert>
#include <charconv>
#include "../../../utils/startswith.hpp"

namespace ECoS {

static const std::string_view endDelimiter = "<END ";

bool parseReply(std::string_view message, Reply& reply)
{
  static const std::string_view startDelimiter = "<REPLY ";

  if(!startsWith(message, startDelimiter))
    return false;

  size_t n = startDelimiter.size();
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
  while((pos = std::min(message.find(',', n), message.find(')', n))) != std::string_view::npos)
  {
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
      reply.lines.emplace_back(&message[n], pos - n);
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

  (void)event;

  return false;
}

}
