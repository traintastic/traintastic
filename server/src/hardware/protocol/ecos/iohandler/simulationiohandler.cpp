/**
 * server/src/hardware/protocol/ecos/iohandler/simulationiohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022,2024 Reinder Feenstra
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

#include "simulationiohandler.hpp"
#include "../kernel.hpp"
#include "../messages.hpp"
#include "../../../../utils/rtrim.hpp"

namespace ECoS {

static bool appendOption(std::string& response, const Simulation::ECoS& ecos, std::string_view option)
{
  if(option == Option::commandstationtype)
  {
    response.append("\"").append(ecos.commandStationType).append("\"");
  }
  else if(option == Option::protocolversion)
  {
    response.append(ecos.protocolVersion);
  }
  else if(option == Option::hardwareversion)
  {
    response.append(ecos.hardwareVersion);
  }
  else if(option == Option::applicationversion)
  {
    response.append(ecos.applicationVersion);
  }
  else if(option == Option::applicationversionsuffix)
  {
    response.append("\"").append(ecos.applicationVersionSuffix).append("\"");
  }
  else if(option == Option::railcom)
  {
    response.append(ecos.railcom ? "1" : "0");
  }
  else if(option == Option::railcomplus)
  {
    response.append(ecos.railcomPlus ? "1" : "0");
  }
  else
  {
    return false;
  }
  return true;
}

static bool appendOption(std::string& response, const Simulation::Switch& sw, std::string_view option)
{
  if(option == Option::name1)
  {
    response.append("\"").append(sw.name1).append("\"");
  }
  else if(option == Option::name2)
  {
    response.append("\"").append(sw.name2).append("\"");
  }
  else if(option == Option::name3)
  {
    response.append("\"").append(sw.name3).append("\"");
  }
  else if(option == Option::addr)
  {
    response.append(std::to_string(sw.address));
  }
  else if(option == Option::addrext)
  {
    response.append(sw.addrext);
  }
  else if(option == Option::symbol)
  {
    response.append(std::to_string(sw.symbol));
  }
  else if(option == Option::type)
  {
    response.append(sw.type);
  }
  else if(option == Option::protocol)
  {
    response.append(sw.protocol);
  }
  else if(option == Option::state)
  {
    response.append(std::to_string(sw.state));
  }
  else if(option == Option::mode)
  {
    response.append(sw.mode);
  }
  else if(option == Option::duration)
  {
    response.append(std::to_string(sw.duration));
  }
  else if(option == Option::variant)
  {
    response.append(std::to_string(sw.variant));
  }
  else
  {
    return false;
  }
  return true;
}


SimulationIOHandler::SimulationIOHandler(Kernel& kernel, const Simulation& simulation)
  : IOHandler(kernel)
  , m_simulation{simulation}
{
}

void SimulationIOHandler::start()
{
  m_kernel.started();
}

bool SimulationIOHandler::send(std::string_view message)
{
  Request request;
  if(!parseRequest(message, request))
    return false;

  if(request.command == Command::request && request.options.size() == 1 && request.options[0] == Option::view)
  {
    return replyOk(message); // notify view active
  }

  if(request.objectId == ObjectId::ecos)
  {
    if(request.command == Command::get)
    {
      std::string response{replyHeader(message)};
      for(auto option : request.options)
      {
        response.append(std::to_string(request.objectId)).append(" ").append(option).append("[");
        if(!appendOption(response, m_simulation.ecos, option))
        {
          return replyErrorUnknownOption(message, option);
        }
        response.append("]\r\n");
      }
      response.append("<END 0 (OK)>\r\n");

      return reply(response);
    }
    if(request.command == Command::set && request.options.size() == 1 && (request.options[0] == Option::stop || request.options[0] == Option::go))
    {
      return replyOk(message);
    }
  }
  else if(request.objectId == ObjectId::locomotiveManager)
  {
    if(request.command == Command::queryObjects)
    {
      std::string response{replyHeader(message)};
      for(const auto& locomotive : m_simulation.locomotives)
      {
        response.append(std::to_string(locomotive.id));
        for(auto option : request.options)
        {
          if(option == Option::protocol)
            response.append(" protocol[").append(toString(locomotive.protocol)).append("]");
          else if(option == Option::addr)
            response.append(" addr[").append(std::to_string(locomotive.address)).append("]");
        }
        response.append("\r\n");
      }
      response.append("<END 0 (OK)>\r\n");

      return reply(response);
    }
  }
  else if(request.objectId == ObjectId::switchManager)
  {
    if(request.command == Command::set && request.options.size() == 1)
    {
      std::string_view option;
      std::string_view value;
      if(parseOptionValue(request.options[0], option, value) && option == Option::switch_)
      {
        return replyOk(message); // notify executed
      }
    }
    else if(request.command == Command::queryObjects)
    {
      std::string response{replyHeader(message)};
      for(const auto& sw : m_simulation.switches)
      {
        response.append(std::to_string(sw.id));
        for(auto option : request.options)
        {
          response.append(" ").append(option).append("[");
          if(!appendOption(response, sw, option))
          {
            assert(false); // FIXME: add error response
          }
          response.append("]");
        }
        response.append("\r\n");
      }
      response.append("<END 0 (OK)>\r\n");
      return reply(response);
    }
  }
  else if(request.objectId == ObjectId::feedbackManager)
  {
    if(request.command == Command::queryObjects)
    {
      std::string response{replyHeader(message)};
      for(const auto& s88 : m_simulation.s88)
      {
        response.append(std::to_string(s88.id));
        for(auto option : request.options)
        {
          if(option == Option::ports)
            response.append(" ports[").append(std::to_string(s88.ports)).append("]");
        }
        response.append("\r\n");
      }
      response.append("<END 0 (OK)>\r\n");

      return reply(response);
    }
  }
  else if(auto itLocomotive = std::find_if(m_simulation.locomotives.begin(), m_simulation.locomotives.end(),
    [id=request.objectId](const auto& v)
    {
      return v.id == id;
    }); itLocomotive != m_simulation.locomotives.end())
  {
    if(request.command == Command::get)
    {
      std::string_view key;
      std::string_view value;
      std::string response{replyHeader(message)};
      for(auto option : request.options)
      {
        if(option == Option::dir)
          response.append(std::to_string(request.objectId)).append(" dir[0]\r\n");
        else if(option == Option::speedStep)
          response.append(std::to_string(request.objectId)).append(" speedstep[0]\r\n");
        else if(parseOptionValue(option, key, value) && key == Option::func)
          response.append(std::to_string(request.objectId)).append(" func[").append(value).append(",0]\r\n");
        else
          assert(false);
      }
      response.append("<END 0 (OK)>\r\n");

      return reply(response);
    }
  }
  else if(auto itSwitch = std::find_if(m_simulation.switches.begin(), m_simulation.switches.end(),
    [id=request.objectId](const auto& v)
    {
      return v.id == id;
    }); itSwitch != m_simulation.switches.end())
  {
    const auto& sw = *itSwitch;

    if(request.command == Command::get)
    {
      std::string response{replyHeader(message)};
      for(auto option : request.options)
      {
        response.append(std::to_string(request.objectId)).append(" ").append(option).append("[");
        if(!appendOption(response, sw, option))
        {
          return replyErrorUnknownOption(message, option);
        }
        response.append("]\r\n");
      }
      response.append("<END 0 (OK)>\r\n");

      return reply(response);
    }
  }
  else if(auto it = std::find_if(m_simulation.s88.begin(), m_simulation.s88.end(),
    [id=request.objectId](const auto& v)
    {
      return v.id == id;
    }); it != m_simulation.s88.end())
  {
    if(request.command == Command::get)
    {
      std::string response{replyHeader(message)};
      for(auto option : request.options)
      {
        if(option == Option::state)
          response.append(std::to_string(request.objectId)).append(" state[0x0]\r\n");
      }
      response.append("<END 0 (OK)>\r\n");

      return reply(response);
    }
  }

  return reply(std::string("<REPLY ").append(message).append(">\r\n<END 999 (Traintastic: no simulation support)>\r\n"));
}

bool SimulationIOHandler::reply(std::string_view message)
{
  // post the reply, so it has some delay
  m_kernel.ioContext().post(
    [this, data=std::string(message)]()
    {
      m_kernel.receive(data);
    });

  return true;
}

bool SimulationIOHandler::replyOk(std::string_view request)
{
  return reply(replyHeader(request).append("<END 0 (OK)>\r\n"));
}

std::string SimulationIOHandler::replyHeader(std::string_view request)
{
  return std::string("<REPLY ").append(rtrim(request, {'\r', '\n'})).append(">\r\n");
}

bool SimulationIOHandler::replyErrorUnknownOption(std::string_view request, std::string_view option)
{
  assert(option.data() >= request.data() && (option.data() + option.size()) <= (request.data() + request.size())); // option view must be within request view
  const auto pos = option.data() - request.data();
  return reply(replyHeader(request).append("<END 11 (unknown option at ").append(std::to_string(1 + pos)).append(")>\r\n"));
}

}
