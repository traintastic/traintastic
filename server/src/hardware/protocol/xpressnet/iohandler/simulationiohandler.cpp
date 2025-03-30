/**
 * server/src/hardware/protocol/xpressnet/iohandler/simulationiohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022,2024-2025 Reinder Feenstra
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
#include "../../../../enum/simulateinputaction.hpp"
#include "../../../../utils/inrange.hpp"

namespace XpressNet {

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

void SimulationIOHandler::setSimulator(std::string hostname, uint16_t port)
{
  assert(!m_simulator);
  m_simulator = std::make_unique<SimulatorIOHandler>(m_kernel.ioContext(), std::move(hostname), port);
}

void SimulationIOHandler::start()
{
  if(m_simulator)
  {
    m_simulator->onPower =
      [this](bool powerOn)
      {
        if(powerOn)
        {
          reply(NormalOperationResumed(), 3);
        }
        else
        {
          reply(TrackPowerOff(), 3);
        }
      };
    m_simulator->onSensorChanged =
      [this](uint16_t /*channel*/, uint16_t address, bool value)
      {
        if(inRange(address, Kernel::inputAddressMin, Kernel::inputAddressMax))
        {
          simulateInputChange(address, value ? SimulateInputAction::SetTrue : SimulateInputAction::SetFalse);
        }
      };
    m_simulator->start();
  }
  m_kernel.started();
}

bool SimulationIOHandler::send(const Message& message)
{
  switch(message.header)
  {
    case 0x21:
      if(message == ResumeOperationsRequest())
      {
        if(m_simulator)
        {
          m_simulator->sendPower(true);
        }
        reply(NormalOperationResumed(), 3);
      }
      else if(message == StopOperationsRequest())
      {
        if(m_simulator)
        {
          m_simulator->sendPower(false);
        }
        reply(TrackPowerOff(), 3);
      }
      break;

    case 0x80:
      if(message == StopAllLocomotivesRequest())
        reply(EmergencyStop(), 3);
      break;
  }

  return true;
}

void SimulationIOHandler::simulateInputChange(uint16_t address, SimulateInputAction action)
{
  assert(inRange(address, Kernel::inputAddressMin, Kernel::inputAddressMax));
  switch(action)
  {
    case SimulateInputAction::SetFalse:
      m_inputs[address] = false;
      break;

    case SimulateInputAction::SetTrue:
      m_inputs[address] = true;
      break;

    case SimulateInputAction::Toggle:
      if(auto it = m_inputs.find(address); it != m_inputs.end())
      {
        it->second = !it->second;
      }
      else
      {
        m_inputs[address] = true;
      }
      break;
  }

  // send FeedbackBroadcast message:
  const uint16_t groupAddress = (address - Kernel::inputAddressMin) >> 2;
  std::byte message[sizeof(FeedbackBroadcast) + sizeof(FeedbackBroadcast::Pair) + 1];
  memset(message, 0, sizeof(message));
  auto* feedbackBroadcast = reinterpret_cast<FeedbackBroadcast*>(&message);
  feedbackBroadcast->header = idFeedbackBroadcast;
  feedbackBroadcast->setPairCount(1);
  auto& pair = feedbackBroadcast->pair(0);
  pair.setGroupAddress(groupAddress);
  pair.setType(FeedbackBroadcast::Pair::Type::FeedbackModule);
  for(uint8_t i = 0; i < 4; i++)
  {
    const uint16_t n = Kernel::inputAddressMin + (groupAddress << 2) + i;
    if(auto it = m_inputs.find(n); it != m_inputs.end())
    {
      pair.setStatus(i, it->second);
    }
  }
  updateChecksum(*feedbackBroadcast);

  reply(*feedbackBroadcast);
}

void SimulationIOHandler::reply(const Message& message)
{
  // post the reply, so it has some delay
  //! \todo better delay simulation? at least xpressnet message transfer time?
  m_kernel.ioContext().post(
    [this, data=copy(message)]()
    {
      m_kernel.receive(*reinterpret_cast<const Message*>(data.get()));
    });
}

void SimulationIOHandler::reply(const Message& message, const size_t count)
{
  for(size_t i = 0; i < count; i++)
    reply(message);
}

}
