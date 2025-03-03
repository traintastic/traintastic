/**
 * server/src/hardware/protocol/loconet/iohandler/simulationiohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2023,2025 Reinder Feenstra
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
#include "../../dcc/dcc.hpp"

namespace LocoNet {

static std::shared_ptr<std::byte[]> copy(const Message& message)
{
  auto* bytes = new std::byte[message.size()];
  std::memcpy(bytes, &message, message.size());
  return std::shared_ptr<std::byte[]>{bytes};
}

static void updateActive(SlotReadData& locoSlot)
{
  locoSlot.setActive(
    (locoSlot.spd != SPEED_STOP && locoSlot.spd != SPEED_ESTOP) ||
    (locoSlot.dirf & (SL_F0 | SL_F4 | SL_F3 | SL_F2 | SL_F1)) != 0 ||
    (locoSlot.snd & (SL_F8 | SL_F7 | SL_F6 | SL_F5)) != 0);
}

SimulationIOHandler::SimulationIOHandler(Kernel& kernel)
  : IOHandler(kernel)
{
  for(uint8_t slot = SLOT_LOCO_MIN; slot <= SLOT_LOCO_MAX; slot++)
    m_locoSlots[slot - SLOT_LOCO_MIN].slot = slot;

  // LNCV modules:
  m_lncvModules.emplace_back(LNCVModule{
    6312, // Uhlenbrock USB LocoNet interface 68610
    false,
    {
      {0, 1},
      {1, 0},
      {2, 4},
      {4, 0},
    }});

  m_lncvModules.emplace_back(LNCVModule{
    6388, // Uhlenbrock S88 LocoNet adaptor 63880
    false,
    {
      {0, 1},
      {1, 0},
      {2, 20},
      {3, 31},
      {4, 1},
    }});
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
          reply(GlobalPowerOn());
        }
        else
        {
          reply(GlobalPowerOff());
        }
      };
    m_simulator->onLocomotiveSpeedDirection =
      [this](DecoderProtocol protocol, uint16_t address, uint8_t speed, Direction direction, bool emergencyStop)
      {
        if((protocol == DecoderProtocol::None || protocol == DecoderProtocol::DCCShort || protocol == DecoderProtocol::DCCLong) &&
            inRange(address, DCC::addressMin, DCC::addressLongMax))
        {
          auto* locoSlot = findSlot(address);
          if(!locoSlot)
          {
            reply(LocoAdr(address));

            locoSlot = getFreeSlot();
            if(locoSlot)
            {
              locoSlot->setBusy(true);
              locoSlot->setAddress(address);
              updateChecksum(*locoSlot);
              reply(*locoSlot);
            }
            else
            {
              reply(LongAck(OPC_LOCO_ADR, 0));
            }
          }

          if(locoSlot)
          {
            if(locoSlot->direction() != direction)
            {
              locoSlot->setDirection(direction);
              LocoDirF locoDirF(direction, locoSlot->f0(), locoSlot->f1(), locoSlot->f2(), locoSlot->f3(), locoSlot->f4());
              locoDirF.slot = locoSlot->slot;
              updateChecksum(locoDirF);
              reply(locoDirF);
            }

            speed = (speed * 126) / std::numeric_limits<uint8_t>::max();
            if((!locoSlot->isEmergencyStop() && emergencyStop) || speed != locoSlot->speed())
            {
              if(emergencyStop)
              {
                locoSlot->spd = SPEED_ESTOP;
              }
              else if(speed == 0)
              {
                locoSlot->spd = 0;
              }
              else
              {
                locoSlot->spd = 1 + speed;
              }
              LocoSpd locoSpd(locoSlot->spd);
              locoSpd.slot = locoSlot->slot;
              updateChecksum(locoSpd);
              reply(locoSpd);
            }
          }
        }
      };
    m_simulator->onSensorChanged =
      [this](uint16_t /*channel*/, uint16_t address, bool value)
      {
        if(inRange(address, Kernel::inputAddressMin, Kernel::inputAddressMax))
        {
          m_kernel.receive(InputRep(address - 1, value));
        }
      };
    m_simulator->start();
  }
  started();
}

void SimulationIOHandler::stop()
{
  if(m_simulator)
  {
    m_simulator->stop();
  }
}

bool SimulationIOHandler::send(const Message& message)
{
  reply(message); // echo message back

  switch(message.opCode)
  {
    case OPC_GPOFF:
      if(m_simulator)
      {
        m_simulator->sendPower(false);
      }
      break;

    case OPC_GPON:
      if(m_simulator)
      {
        m_simulator->sendPower(true);
      }
      break;

    case OPC_UNLINK_SLOTS:
      break; // unimplemented

    case OPC_LINK_SLOTS:
      break; // unimplemented

    case OPC_MOVE_SLOTS:
      break; // unimplemented

    case OPC_RQ_SL_DATA:
    {
      const auto& requestSlotData = static_cast<const RequestSlotData&>(message);
      if(isLocoSlot(requestSlotData.slot))
      {
        auto& locoSlot = m_locoSlots[requestSlotData.slot - SLOT_LOCO_MIN];
        updateChecksum(locoSlot);
        reply(locoSlot);
      }
      break;
    }
    case OPC_SW_REQ:
    {
      if(m_simulator)
      {
        const auto& switchRequest = static_cast<const SwitchRequest&>(message);
        uint8_t state = 0;
        if(switchRequest.on())
        {
          state = switchRequest.dir() ? 2 : 1;
        }
        m_simulator->sendAccessorySetState(0, switchRequest.address(), state);
      }
      break;
    }
    case OPC_SW_STATE:
      break; // unimplemented

    case OPC_SW_ACK:
      break; // unimplemented

    case OPC_LOCO_ADR:
    {
      const auto& locoAdr = static_cast<const LocoAdr&>(message);

      if(auto* slot = findSlot(locoAdr.address()))
      {
        updateChecksum(*slot);
        reply(*slot);
        return true;
      }

      if(auto* slot = getFreeSlot())
      {
        slot->setBusy(true);
        slot->setAddress(locoAdr.address());
        updateChecksum(*slot);
        reply(*slot);
        return true;
      }

      // no free slot
      reply(LongAck(message.opCode, 0));

      break;
    }
    case OPC_IMM_PACKET:
    {
      if(Uhlenbrock::LNCVStart::check(message))
      {
        const auto lncvStart = static_cast<const Uhlenbrock::LNCVStart&>(message);

        for(auto& module : m_lncvModules)
        {
          if(lncvStart.moduleId() == module.id && (lncvStart.address() == module.address() || lncvStart.address() == LNCVModule::broadcastAddress))
          {
            module.programmingModeActive = true;
            reply(Uhlenbrock::LNCVReadResponse(lncvStart.moduleId(), LNCVModule::lncvAddress, module.address()));
          }
        }
      }
      else if(Uhlenbrock::LNCVRead::check(message))
      {
        const auto lncvRead = static_cast<const Uhlenbrock::LNCVRead&>(message);

        for(auto& module : m_lncvModules)
        {
          if(lncvRead.moduleId() == module.id && module.programmingModeActive)
          {
            if(auto it = module.lncvs.find(lncvRead.lncv()); it != module.lncvs.end())
            {
              reply(Uhlenbrock::LNCVReadResponse(lncvRead.moduleId(), lncvRead.lncv(), it->second));
            }
          }
        }
      }
      else if(Uhlenbrock::LNCVWrite::check(message))
      {
        const auto lncvWrite = static_cast<const Uhlenbrock::LNCVWrite&>(message);

        for(auto& module : m_lncvModules)
        {
          if(lncvWrite.moduleId() == module.id && module.programmingModeActive)
          {
            if(auto it = module.lncvs.find(lncvWrite.lncv()); it != module.lncvs.end())
            {
              it->second = lncvWrite.value();
              reply(LongAck(lncvWrite.opCode, 0x7F));
            }
          }
        }
      }
      else if(Uhlenbrock::LNCVStop::check(message))
      {
        const auto lncvStop = static_cast<const Uhlenbrock::LNCVStop&>(message);

        for(auto& module : m_lncvModules)
        {
          if(lncvStop.moduleId() == module.id)
          {
            module.programmingModeActive = false;
          }
        }
      }
      else if(isSignatureMatch<ImmPacket>(message))
      {
        reply(LongAck(message.opCode, 0x7F)); // accept, not limited
      }
      break;
    }
    case OPC_WR_SL_DATA:
      reply(LongAck(message.opCode, 0x7F)); // slot write successful
      break;

    // no response:
    case OPC_LOCO_SPD:
    {
      const auto& locoSpd = static_cast<const LocoSpd&>(message);
      if(isLocoSlot(locoSpd.slot))
      {
        auto& locoSlot = m_locoSlots[locoSpd.slot - SLOT_LOCO_MIN];
        locoSlot.spd = locoSpd.speed;
        updateActive(locoSlot);
        simulatorSendLocoSlot(locoSlot);
      }
      break;
    }
    case OPC_LOCO_DIRF:
    {
      const auto& locoDirF = static_cast<const LocoDirF&>(message);
      if(isLocoSlot(locoDirF.slot))
      {
        auto& locoSlot = m_locoSlots[locoDirF.slot - SLOT_LOCO_MIN];
        const auto prevDirection = locoSlot.direction();
        locoSlot.dirf = locoDirF.dirf;
        updateActive(locoSlot);
        if(prevDirection != locoSlot.direction())
        {
          simulatorSendLocoSlot(locoSlot);
        }
      }
      break;
    }
    case OPC_LOCO_SND:
    {
      const auto& locoSnd = static_cast<const LocoSnd&>(message);
      if(isLocoSlot(locoSnd.slot))
      {
        auto& locoSlot = m_locoSlots[locoSnd.slot - SLOT_LOCO_MIN];
        locoSlot.snd = locoSnd.snd;
        updateActive(locoSlot);
      }
      break;
    }

    case OPC_BUSY:
    case OPC_IDLE:
    case OPC_LOCO_F9F12:
    case OPC_SW_REP:
    case OPC_INPUT_REP:
    case OPC_LONG_ACK:
    case OPC_SLOT_STAT1:
    case OPC_CONSIST_FUNC:
    case OPC_MULTI_SENSE:
    case OPC_D4:
    case OPC_MULTI_SENSE_LONG:
    case OPC_E4:
    case OPC_PEER_XFER:
    case OPC_SL_RD_DATA:
      assert(!hasResponse(message)); // no response
      break;
  }

  return true;
}

void SimulationIOHandler::reply(const Message& message)
{
  // post the reply, so it has some delay
  //! \todo better delay simulation? at least loconet message transfer time?
  m_kernel.ioContext().post(
    [this, data=copy(message)]()
    {
      m_kernel.receive(*reinterpret_cast<const Message*>(data.get()));
    });
}

SlotReadData* SimulationIOHandler::findSlot(uint16_t address)
{
  const auto it = std::find_if(m_locoSlots.begin(), m_locoSlots.end(),
    [address](const auto& locoSlot)
    {
      return locoSlot.address() == address;
    });

  return it != m_locoSlots.end() ? &*it : nullptr;
}

SlotReadData* SimulationIOHandler::getFreeSlot()
{
  const auto it = std::find_if(m_locoSlots.begin(), m_locoSlots.end(),
    [](const auto& locoSlot)
    {
      return locoSlot.isFree();
    });

  return it != m_locoSlots.end() ? &*it : nullptr;
}

void SimulationIOHandler::simulatorSendLocoSlot(const SlotReadData& locoSlot)
{
  if(!m_simulator)
  {
    return;
  }

  m_simulator->sendLocomotiveSpeedDirection(
    DCC::getProtocol(locoSlot.address()),
    locoSlot.address(),
    (locoSlot.speed() * 255) / 126,
    locoSlot.direction(),
    locoSlot.isEmergencyStop()
  );
}

}
