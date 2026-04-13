/**
 * server/src/hardware/protocol/xpressnet/iohandler/simulationiohandler.cpp
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

void SimulationIOHandler::start()
{
  m_kernel.started();
}

bool SimulationIOHandler::send(const Message& message)
{
  switch(message.header)
  {
    case STOP_REQUEST:
    {
      if(message == ResumeOperationsRequest())
        reply(NormalOperationResumed(), 3);
      else if(message == StopOperationsRequest())
        reply(TrackPowerOff(), 3);
      break;
    }
    case SET_STOP_LOCO:
    {
      if(message == StopAllLocomotivesRequest())
        reply(EmergencyStop(), 3);
      break;
    }
    case GET_LOCO_INFO:
    {
      const auto& locomotiveInstruction = static_cast<const LocomotiveInstruction&>(message);
      switch(locomotiveInstruction.identification)
      {
        case 0:
        case idQueryFuncGroup1to3:
        {
          Locomotive& loco = m_locomotives[locomotiveInstruction.address()];
          reply(loco.info);
          break;
        }
        case idQueryFuncGroup4:
        {
          Locomotive& loco = m_locomotives[locomotiveInstruction.address()];
          reply(loco.func13);
          break;
        }
        case idQueryFuncGroup5above:
        {
          Locomotive& loco = m_locomotives[locomotiveInstruction.address()];
          reply(loco.func29);
          break;
        }
      }
      break;
    }
    case SET_LOCO:
    {
      const auto& locomotiveInstruction = static_cast<const LocomotiveInstruction&>(message);
      switch(locomotiveInstruction.identification)
      {
        case idSetSpeed14:
        case idSetSpeed27:
        case idSetSpeed28:
        case idSetSpeed128:
        {
          speedAndDirectionInstruction(static_cast<const SpeedAndDirectionInstruction&>(locomotiveInstruction));
          break;
        }
        default:
        {
          const auto& funcInstruction = static_cast<const FunctionInstructionGroup&>(message);
          const uint8_t group = funcInstruction.getGroup();
          if(group == 0)
            break;

          Locomotive& loco = m_locomotives[funcInstruction.address()];
          loco.info.setBusy(false);
          if(group <= 3)
          {
            for(int i = 0; i <= 12; i++)
              loco.info.setFunction(i, funcInstruction.getFunction(i));
          }
          else if(group == 4)
          {
            for(int i = 13; i <= 28; i++)
              loco.func13.setFunction(i, funcInstruction.getFunction(i));
            loco.func13.updateChecksum();
          }
          else
          {
            for(int i = 29; i <= 68; i++)
              loco.func29.setFunction(i, funcInstruction.getFunction(i));
            loco.func29.updateChecksum();

            if(loco.func29.getFunction(68))
            {
              // Simulate external change when setting function 68
              m_kernel.ioContext().post(
                [this, address=funcInstruction.address()]()
                {
                  Locomotive& loco2 = m_locomotives[address];
                  loco2.info.setBusy(true);
                  loco2.info.updateChecksum();
                  m_kernel.receive(LocomotiveBusy(address));
                });
            }
          }
          loco.info.updateChecksum();
          break;
        }
      }
      break;
    }
  }

  return true;
}

void SimulationIOHandler::reply(const Message& message)
{
  // post the reply, so it has some delay
  //! \todo better delay simulation? at least xpressnet message transfer time?
  boost::asio::post(m_kernel.ioContext(), 
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

void SimulationIOHandler::speedAndDirectionInstruction(const SpeedAndDirectionInstruction& message)
{
  Locomotive& loco = m_locomotives[message.address()];
  loco.info.setBusy(true);
  loco.info.setDirection(message.direction());
  loco.info.setSpeedSteps(message.speedSteps());
  if(message.isEmergencyStop())
    loco.info.setEmergencyStop();
  else
    loco.info.setSpeedStep(message.speedStep());

  if(message.speedSteps() == 14)
  {
    const auto& setSpeed14 = static_cast<const SpeedAndDirectionInstruction14&>(message);
    loco.info.setFunction(0, setSpeed14.getFl());
  }
  loco.info.updateChecksum();
}

}
