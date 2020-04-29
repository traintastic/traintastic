/**
 * server/src/hardware/protocol/loconet.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
 *
 * Portions Copyright (C) Digitrax Inc.
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_HPP

#include "../../core/subobject.hpp"
#include "../../core/property.hpp"
//#include <cstdint>
//#include <cassert>

class LocoNetInput;

namespace Protocol {

class LocoNet : public SubObject
{
  //friend class LocoNetInput;

  public:
    struct Message;

    static uint8_t calcChecksum(const Message& msg);
    static bool isChecksumValid(const Message& msg);

    enum OpCode : uint8_t
    {
      // 2 byte message opcodes:
      OPC_IDLE = 0x85,
      OPC_GPON = 0x83,
      OPC_GPOFF = 0x82,
      OPC_BUSY = 0x81,

      // 4 byte message opcodes:
      OPC_LOCO_SPD = 0xA0,
      OPC_INPUT_REP = 0xB2,

      // 6 byte message opcodes:
      OPC_MULTI_SENSE = 0xD0,
    };

    struct Message
    {
      OpCode opCode;

      uint8_t size() const
      {
        switch(opCode & 0xE0)
        {
          case 0x80: // 1 0 0 F D C B A
            return 2;

          case 0xA0: // 1 0 1 F D C B A
            return 4;

          case 0xC0: // 1 1 0 F D C B A
            return 6;

          case 0xE0: // 1 1 1 F D C B A => length in next byte
            return reinterpret_cast<const uint8_t*>(this)[1];

          default:
            assert(false);
            return 0;
        }
      }
    };

    struct LocoSpd : Message
    {
      uint8_t slot;
      uint8_t speed;
      uint8_t checksum;

      LocoSpd(uint8_t _slot, uint8_t _speed) :
        slot{_slot},
        speed{_speed}
      {
        checksum = calcChecksum(*this);
      }
    };
    static_assert(sizeof(LocoSpd) == 4);

    /*

    2020-02-04 21:27:59.123954 [debug]    cs1: unknown message: dataLen=0x0008, header=0x00a0, data=b2 11 70 2c
    2020-02-04 21:27:59.413558 [debug]    cs1: unknown message: dataLen=0x0008, header=0x00a0, data=b2 19 40 14
    2020-02-04 21:28:00.046282 [debug]    cs1: unknown message: dataLen=0x0008, header=0x00a0, data=b2 11 50 0c
    2020-02-04 21:28:00.433662 [debug]    cs1: unknown message: dataLen=0x0008, header=0x00a0, data=b2 11 60 3c
    2020-02-04 21:28:02.502012 [debug]    cs1: unknown message: dataLen=0x0008, header=0x00a0, data=b2 09 70 34
    2020-02-04 21:28:02.913595 [debug]    cs1: unknown message: dataLen=0x0008, header=0x00a0, data=b2 11 40 1c
    2020-02-04 21:28:03.629638 [debug]    cs1: unknown message: dataLen=0x0008, header=0x00a0, data=b2 09 50 14
    2020-02-04 21:28:03.937476 [debug]    cs1: unknown message: dataLen=0x0008, header=0x00a0, data=b2 09 60 24

     */



    struct InputRep : Message
    {
      uint8_t in1;
      uint8_t in2;
      uint8_t checksum;

      inline uint16_t address() const
      {
        return in1 & 0x7F + static_cast<uint16_t>(in2 & 0x0F) << 7;
      }

      inline bool isSwitchInput() const
      {
        return in2 & 0x20;
      }

      inline bool isAuxInput() const
      {
        return !isSwitchInput();
      }

      inline bool value() const
      {
        return in2 & 0x10;
      }
    };
    static_assert(sizeof(InputRep) == 4);



    // d0 00 42 20 13 5e

    struct MultiSense : Message
    {
      uint8_t data1;
      uint8_t data2;
      uint8_t data3;
      uint8_t data4;
      uint8_t checksum;
    };
    static_assert(sizeof(MultiSense) == 6);

  protected:
    std::function<bool(const Message&)> m_send;
    std::atomic_bool m_debugLog;
    std::unordered_map<uint16_t, std::shared_ptr<LocoNetInput>> m_inputs;



  public://protected:
    bool isInputAddressAvailable(uint16_t address);
    bool addInput(const std::shared_ptr<LocoNetInput>& input);
    void removeInput(const std::shared_ptr<LocoNetInput>& input);

  public:
    CLASS_ID("protocol.loconet")

    Property<bool> debugLog;

    LocoNet(Object& _parent, const std::string& parentPropertyName, std::function<bool(const Message&)> send);

    bool send(const Message& msg) { return m_send(msg); }
    void receive(const Message& msg);
};

}

#endif



