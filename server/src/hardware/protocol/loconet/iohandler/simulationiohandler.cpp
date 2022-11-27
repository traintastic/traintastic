/**
 * server/src/hardware/protocol/loconet/iohandler/simulationiohandler.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

bool SimulationIOHandler::send(const Message& message)
{
  reply(message); // echo message back

  switch(message.opCode)
  {
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
    case OPC_SW_STATE:
      break; // unimplemented

    case OPC_SW_ACK:
      break; // unimplemented

    case OPC_LOCO_ADR:
    {
      const auto& locoAdr = static_cast<const LocoAdr&>(message);

      // find slot for address
      {
        auto it = std::find_if(m_locoSlots.begin(), m_locoSlots.end(), // NOLINT [readability-qualified-auto]
          [address=locoAdr.address()](const auto& locoSlot)
          {
            return locoSlot.address() == address;
          });

        if(it != m_locoSlots.end())
        {
          updateChecksum(*it);
          reply(*it);
          return true;
        }
      }

      // find a free slot
      {
        auto it = std::find_if(m_locoSlots.begin(), m_locoSlots.end(), // NOLINT [readability-qualified-auto]
          [](const auto& locoSlot)
          {
            return locoSlot.isFree();
          });

        if(it != m_locoSlots.end())
        {
          it->setBusy(true);
          it->setAddress(locoAdr.address());
          updateChecksum(*it);
          reply(*it);
          return true;
        }
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
      }
      break;
    }
    case OPC_LOCO_DIRF:
    {
      const auto& locoDirF = static_cast<const LocoDirF&>(message);
      if(isLocoSlot(locoDirF.slot))
      {
        auto& locoSlot = m_locoSlots[locoDirF.slot - SLOT_LOCO_MIN];
        locoSlot.dirf = locoDirF.dirf;
        updateActive(locoSlot);
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
    case OPC_GPOFF:
    case OPC_GPON:
    case OPC_IDLE:
    case OPC_LOCO_F9F12:
    case OPC_SW_REQ:
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

}
