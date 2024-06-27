/**
 * server/src/hardware/protocol/traintasticdiy/iohandler/simulationiohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2024 Reinder Feenstra
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
#include <version.hpp>

namespace TraintasticDIY {

static std::shared_ptr<std::byte[]> copy(const Message& message)
{
  auto* bytes = new std::byte[message.size()];
  std::memcpy(bytes, &message, message.size());
  return std::shared_ptr<std::byte[]>{bytes};
}

SimulationIOHandler::SimulationIOHandler(Kernel& kernel)
  : IOHandler(kernel)
{
}

void SimulationIOHandler::start()
{
  m_kernel.started();
}

bool SimulationIOHandler::send(const Message& message)
{
  switch(message.opCode)
  {
    case OpCode::Heartbeat:
      reply(Heartbeat());
      break;

    case OpCode::GetInputState:
    {
      const auto& getInputState = static_cast<const GetInputState&>(message);
      reply(SetInputState(getInputState.address(), InputState::Invalid));
      break;
    }
    case OpCode::GetOutputState:
    {
      const auto& getOutputState = static_cast<const GetOutputState&>(message);
      reply(SetOutputState(getOutputState.address(), OutputState::Invalid));
      break;
    }
    case OpCode::SetOutputState:
    {
#ifndef NDEBUG
      const auto& setOutputState = static_cast<const SetOutputState&>(message);
      assert(setOutputState.state == OutputState::False || setOutputState.state == OutputState::True);
#endif
      reply(message);
      break;
    }
    case OpCode::ThrottleSubUnsub:
    {
      // TODO
      break;
    }
    case OpCode::ThrottleSetFunction:
    {
      // TODO
      break;
    }
    case OpCode::ThrottleSetSpeedDirection:
    {
      // TODO
      break;
    }
    case OpCode::GetFeatures:
    {
      reply(Features(FeatureFlags1::Input | FeatureFlags1::Output));
      break;
    }
    case OpCode::GetInfo:
    {
      constexpr std::string_view text{"Traintastic DIY simulator v" TRAINTASTIC_VERSION};
      static_assert(text.size() <= 255);
      auto info = std::make_unique<std::byte[]>(sizeof(InfoBase) + text.size() + sizeof(Checksum));
      auto& infoBase = *reinterpret_cast<InfoBase*>(info.get());
      infoBase.opCode = OpCode::Info;
      infoBase.length = text.length();
      std::memcpy(info.get() + sizeof(InfoBase), text.data(), text.size());
      updateChecksum(infoBase);
      reply(infoBase);
      break;
    }
    case OpCode::SetInputState:
    case OpCode::Features:
    case OpCode::Info:
      assert(false); // only send by device
      break;
  }

  return true;
}

void SimulationIOHandler::reply(const Message& message)
{
  // post the reply, so it has some delay
  //! \todo better delay simulation? at least message transfer time?
  m_kernel.ioContext().post(
    [this, data=copy(message)]()
    {
      m_kernel.receive(*reinterpret_cast<const Message*>(data.get()));
    });
}

}
