/**
 * server/src/hardware/protocol/traintasticcs/iohandler/simulationiohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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
#include <tcb/span.hpp>
#include "../kernel.hpp"
#include "../messages.hpp"
#include "../config.hpp"
#include "../../../../enum/simulateinputaction.hpp"
#include "../../../../utils/inrange.hpp"

namespace TraintasticCS {

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
  switch(message.command)
  {
    case Command::Reset:
      if(message.length != 0)
      {
        reply(Error(message.command, ErrorCode::InvalidCommandPayload));
      }
      else
      {
        m_initXpressNet = false;
        m_s88.reset();
        reply(ResetOk());
      }
      return true;

    case Command::Ping:
      if(message.length != 0)
      {
        reply(Error(message.command, ErrorCode::InvalidCommandPayload));
      }
      else
      {
        reply(Pong());
      }
      return true;

    case Command::GetInfo:
      if(message.length != 0)
      {
        reply(Error(message.command, ErrorCode::InvalidCommandPayload));
      }
      else
      {
        reply(Info(Board::TraintasticCS, 0, 1, 0));
      }
      return true;

    case Command::InitXpressNet:
      if(message.length != 0)
      {
        reply(Error(message.command, ErrorCode::InvalidCommandPayload));
      }
      else if(m_initXpressNet)
      {
        reply(Error(message.command, ErrorCode::AlreadyInitialized));
      }
      else
      {
        m_initXpressNet = true;
        reply(InitXpressNetOk());
      }
      return true;

    case Command::InitS88:
    {
      const auto& initS88 = static_cast<const InitS88&>(message);
      if(message.size() != sizeof(InitS88) ||
          initS88.moduleCount < Config::S88::moduleCountMin ||
          initS88.moduleCount > Config::S88::moduleCountMax ||
          initS88.clockFrequency < Config::S88::clockFrequencyMin ||
          initS88.clockFrequency > Config::S88::clockFrequencyMax)
      {
        reply(Error(message.command, ErrorCode::InvalidCommandPayload));
      }
      else if(m_s88.enabled)
      {
        reply(Error(message.command, ErrorCode::AlreadyInitialized));
      }
      else
      {
        m_s88.init(initS88.moduleCount);
        reply(InitS88Ok());
      }
      return true;
    }
    case Command::InitLocoNet:
      if(message.size() != sizeof(InitLocoNet))
      {
        reply(Error(message.command, ErrorCode::InvalidCommandPayload));
      }
      else if(m_loconet.enabled)
      {
        reply(Error(message.command, ErrorCode::AlreadyInitialized));
      }
      else
      {
        m_loconet.init();
        reply(InitLocoNetOk());
      }
      return true;

    case Command::ResetOk:
    case Command::Pong:
    case Command::Info:
    case Command::InitXpressNetOk:
    case Command::InitS88Ok:
    case Command::InitLocoNetOk:
    case Command::InputStateChanged:
    case Command::ThrottleSetSpeedDirection:
    case Command::ThrottleSetFunctions:
    case Command::Error:
      assert(false); // only send by device
      break;
  }

  reply(Error(message.command, ErrorCode::InvalidCommand));
  return true;
}

void SimulationIOHandler::inputSimulateChange(InputChannel channel, uint16_t address, SimulateInputAction action)
{
  tcb::span<InputState> states;

  switch(channel)
  {
    case InputChannel::LocoNet:
      states = m_loconet.inputStates;
      break;

    case InputChannel::XpressNet:
      return; //! \todo implement

    case InputChannel::S88:
      states = m_s88.states;
      break;
  }
  assert(!states.empty());

  if(inRange<uint16_t>(address, 1, states.size())) /*[[likely]]*/
  {
    InputState newValue = InputState::Unknown;
    switch(action)
    {
      case SimulateInputAction::SetFalse:
        newValue = InputState::Low;
        break;

      case SimulateInputAction::SetTrue:
        newValue = InputState::High;
        break;

      case SimulateInputAction::Toggle:
        newValue = (states[address - 1] == InputState::High) ? InputState::Low : InputState::High;
        break;
    }
    assert(newValue == InputState::Low || newValue == InputState::High);
    if(states[address - 1] != newValue)
    {
      states[address - 1] = newValue;
      reply(InputStateChanged(channel, address, newValue));
    }
  }
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


void SimulationIOHandler::LocoNet::init()
{
  enabled = true;
  std::fill(inputStates.begin(), inputStates.end(), InputState::Unknown);
}

void SimulationIOHandler::LocoNet::reset()
{
  enabled = false;
}


void SimulationIOHandler::S88::init(uint8_t moduleCount)
{
  enabled = true;
  states.resize(moduleCount * 8);
  std::fill(states.begin(), states.end(), InputState::Unknown);
}

void SimulationIOHandler::S88::reset()
{
  enabled = false;
  states.clear();
}

}
