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
        {
          Locomotive& loco = m_locomotives[locomotiveInstruction.address()];
          reply(loco.info);
          break;
        }
        case idQueryFuncGroup4and5:
        {
          Locomotive& loco = m_locomotives[locomotiveInstruction.address()];
          reply(loco.func13);
          break;
        }
        case idQueryFuncGroup6above:
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
        case idSetFuncGroup4_Roco:
        {
          const auto& funcInstruction = static_cast<const RocoMultiMAUS::FunctionInstructionF13F20&>(message);

          Locomotive& loco = m_locomotives[funcInstruction.address()];
          loco.info.setBusy(false);
          loco.info.updateChecksum();

          for(int i = 13; i <= 20; i++)
            loco.func13.setFunction(i, funcInstruction.getFunction(i));
          loco.func13.updateChecksum();
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
            uint8_t i = FunctionInstructionGroup::getMinFunctionIndex(group);
            const uint8_t maxFuncIndex = FunctionInstructionGroup::getMaxFunctionIndex(group);
            for(; i <= maxFuncIndex; i++)
              loco.info.setFunction(i, funcInstruction.getFunction(i));
          }
          else if(group == 4 || group == 5)
          {
            uint8_t i = FunctionInstructionGroup::getMinFunctionIndex(group);
            const uint8_t maxFuncIndex = FunctionInstructionGroup::getMaxFunctionIndex(group);
            for(; i <= maxFuncIndex; i++)
              loco.func13.setFunction(i, funcInstruction.getFunction(i));
            loco.func13.updateChecksum();
          }
          else
          {
            uint8_t i = FunctionInstructionGroup::getMinFunctionIndex(group);
            const uint8_t maxFuncIndex = FunctionInstructionGroup::getMaxFunctionIndex(group);
            for(; i <= maxFuncIndex; i++)
              loco.func29.setFunction(i, funcInstruction.getFunction(i));
            loco.func29.updateChecksum();
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
  loco.info.setBusy(false);
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
