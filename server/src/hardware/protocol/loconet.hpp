/**
 * server/src/hardware/protocol/loconet.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

/**
 * Portions Copyright (C) Digitrax Inc.
 *
 * LocoNet is a registered trademark of DigiTrax, Inc.
 */

/**
 * OPC_MULTI_SENSE and OPC_MULTI_SENSE_LONG message format
 * based on reverse engineering, see loconet.md
 */

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_HPP

#include <vector>
#include "../../core/subobject.hpp"
#include "../../core/property.hpp"
#include <enum/direction.hpp>
#include "../../enum/loconetcommandstation.hpp"
//#include <cstdint>
//#include <cassert>
#include "../../hardware/decoder/decoderchangeflags.hpp"

namespace Hardware {
  class Decoder;
}

class LocoNetInput;

namespace Protocol {

class LocoNet : public SubObject
{
  //friend class LocoNetInput;

  public:
    struct Message;

    static uint8_t calcChecksum(const Message& msg);
    static void updateChecksum(Message& msg);
    static bool isChecksumValid(const Message& msg);

    static constexpr uint8_t SLOT_LOCO_MIN = 1;
    static constexpr uint8_t SLOT_LOCO_MAX = 119;
    static constexpr uint8_t SLOT_FAST_CLOCK = 123;
    static constexpr uint8_t SLOT_PROGRAMMING_TRACK = 124;
    static constexpr uint8_t SLOT_UNKNOWN = 255; //!< placeholder to indicate invalid slot

    static constexpr uint8_t SPEED_STOP = 0;
    static constexpr uint8_t SPEED_ESTOP = 1;
    static constexpr uint8_t SPEED_MAX = 127;

    static constexpr uint8_t SL_CONUP = 0x40;
    static constexpr uint8_t SL_BUSY = 0x20;
    static constexpr uint8_t SL_ACTIVE = 0x10;
    static constexpr uint8_t SL_CONDN = 0x08;

    static constexpr uint8_t SL_DIR = 0x20;
    static constexpr uint8_t SL_F0 = 0x10;
    static constexpr uint8_t SL_F4 = 0x08;
    static constexpr uint8_t SL_F3 = 0x04;
    static constexpr uint8_t SL_F2 = 0x02;
    static constexpr uint8_t SL_F1 = 0x01;

    static constexpr uint8_t SL_F8 = 0x08;
    static constexpr uint8_t SL_F7 = 0x04;
    static constexpr uint8_t SL_F6 = 0x02;
    static constexpr uint8_t SL_F5 = 0x01;

    static constexpr uint8_t MULTI_SENSE_TYPE_MASK = 0xE0;
    static constexpr uint8_t MULTI_SENSE_TYPE_TRANSPONDER_GONE = 0x00;
    static constexpr uint8_t MULTI_SENSE_TYPE_TRANSPONDER_PRESENT = 0x20;
    static constexpr uint8_t MULTI_SENSE_TRANSPONDER_ADDRESS_SHORT = 0xFD;

    enum OpCode : uint8_t
    {
      // 2 byte message opcodes:
      OPC_BUSY = 0x81,
      OPC_GPOFF = 0x82,
      OPC_GPON = 0x83,
      OPC_IDLE = 0x85,

      // 4 byte message opcodes:
      OPC_LOCO_SPD = 0xA0,
      OPC_LOCO_DIRF = 0xA1,
      OPC_LOCO_SND = 0xA2,
      OPC_SW_REQ = 0xB0,
      OPC_SW_REP = 0xB1,
      OPC_INPUT_REP = 0xB2,
      OPC_LONG_ACK = 0xB4,
      OPC_SLOT_STAT1 = 0xB5,
      OPC_CONSIST_FUNC = 0xB6,
      OPC_UNLINK_SLOTS = 0xB8,
      OPC_LINK_SLOTS = 0xB9,
      OPC_MOVE_SLOTS = 0xBA,
      OPC_RQ_SL_DATA = 0xBB,
      OPC_SW_STATE = 0xBC,
      OPC_SW_ACK = 0xBD,
      OPC_LOCO_ADR = 0xBF,

      // 6 byte message opcodes:
      OPC_MULTI_SENSE = 0xD0,

      // variable byte message opcodes:
      OPC_MULTI_SENSE_LONG = 0XE0,
      OPC_PEER_XFER = 0xE5,
      OPC_SL_RD_DATA = 0xE7,
      OPC_IMM_PACKET = 0xED,
      OPC_WR_SL_DATA = 0xEF,
    };

    struct Message
    {
      OpCode opCode;

      Message()
      {
      }

      Message(OpCode _opCode) :
        opCode{_opCode}
      {
      }

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
            //assert(false);
            return 0;
        }
      }
    };

    struct SlotMessage : Message
    {
      uint8_t slot;

      SlotMessage(OpCode _opCode, uint8_t _slot) :
        Message{_opCode},
        slot{_slot}
      {
      }
    };

    struct Idle : Message
    {
      uint8_t checksum;

      Idle() :
        Message{OPC_IDLE},
        checksum{0x7A}
      {
      }
    };
    static_assert(sizeof(Idle) == 2);

    struct GlobalPowerOn : Message
    {
      uint8_t checksum;

      GlobalPowerOn() :
        Message{OPC_GPON},
        checksum{0x7C}
      {
      }
    };
    static_assert(sizeof(GlobalPowerOn) == 2);

    struct GlobalPowerOff : Message
    {
      uint8_t checksum;

      GlobalPowerOff() :
        Message{OPC_GPOFF},
        checksum{0x7D}
      {
      }
    };
    static_assert(sizeof(GlobalPowerOff) == 2);

    struct Busy : Message
    {
      uint8_t checksum;

      Busy() :
        Message{OPC_BUSY},
        checksum{0x7E}
      {
      }
    };
    static_assert(sizeof(Busy) == 2);



    struct LocoAdr : Message
    {
      uint8_t addressHigh;
      uint8_t addressLow;
      uint8_t checksum;

      LocoAdr(uint16_t address) :
        Message{OPC_LOCO_ADR},
        addressHigh{static_cast<uint8_t>(address >> 7)},
        addressLow{static_cast<uint8_t>(address & 0x7F)}
      {
        checksum = calcChecksum(*this);
      }
    };
    static_assert(sizeof(LocoAdr) == 4);

    struct LocoSpd : SlotMessage
    {
      uint8_t speed;
      uint8_t checksum;

      LocoSpd(uint8_t _speed) :
        SlotMessage{OPC_LOCO_SPD, SLOT_UNKNOWN},
        speed{_speed}
      {
        checksum = calcChecksum(*this);
      }
    };
    static_assert(sizeof(LocoSpd) == 4);

    struct LocoDirF : SlotMessage
    {
      uint8_t dirf;
      uint8_t checksum;

      LocoDirF(Direction direction, bool f0, bool f1, bool f2, bool f3, bool f4) :
        SlotMessage{OPC_LOCO_DIRF, SLOT_UNKNOWN},
        dirf{0}
      {
        if(direction == Direction::Forward)
          dirf |= SL_DIR;
        if(f0)
          dirf |= SL_F0;
        if(f1)
          dirf |= SL_F1;
        if(f2)
          dirf |= SL_F2;
        if(f3)
          dirf |= SL_F3;
        if(f4)
          dirf |= SL_F4;
      }

      inline Direction direction() const
      {
        return (dirf & SL_DIR) ? Direction::Forward : Direction::Reverse;
      }

      inline void setDirection(Direction value)
      {
        if(value == Direction::Forward)
          dirf |= SL_DIR;
        else
          dirf &= ~SL_DIR;
      }

      inline bool f0() const
      {
        return dirf & SL_F0;
      }

      inline void setF0(bool value)
      {
        if(value)
          dirf |= SL_F0;
        else
          dirf &= ~SL_F0;
      }

      inline bool f1() const
      {
        return dirf & SL_F1;
      }

      inline void setF1(bool value)
      {
        if(value)
          dirf |= SL_F1;
        else
          dirf &= ~SL_F1;
      }

      inline bool f2() const
      {
        return dirf & SL_F2;
      }

      inline void setF2(bool value)
      {
        if(value)
          dirf |= SL_F2;
        else
          dirf &= ~SL_F2;
      }

      inline bool f3() const
      {
        return dirf & SL_F3;
      }

      inline void setF3(bool value)
      {
        if(value)
          dirf |= SL_F3;
        else
          dirf &= ~SL_F3;
      }

      inline bool f4() const
      {
        return dirf & SL_F4;
      }

      inline void setF4(bool value)
      {
        if(value)
          dirf |= SL_F4;
        else
          dirf &= ~SL_F4;
      }
    };
    static_assert(sizeof(LocoDirF) == 4);

    struct LocoSnd : SlotMessage
    {
      uint8_t snd;
      uint8_t checksum;

      LocoSnd(bool f5, bool f6, bool f7, bool f8) :
        SlotMessage{OPC_LOCO_DIRF, SLOT_UNKNOWN},
        snd{0}
      {
        if(f5)
          snd |= SL_F5;
        if(f6)
          snd |= SL_F6;
        if(f7)
          snd |= SL_F7;
        if(f8)
          snd |= SL_F8;

        checksum = calcChecksum(*this);
      }

      inline bool f5() const
      {
        return snd & SL_F5;
      }

      inline void setF5(bool value)
      {
        if(value)
          snd |= SL_F5;
        else
          snd &= ~SL_F5;
      }

      inline bool f6() const
      {
        return snd & SL_F6;
      }

      inline void setF6(bool value)
      {
        if(value)
          snd |= SL_F6;
        else
          snd &= ~SL_F6;
      }

      inline bool f7() const
      {
        return snd & SL_F7;
      }

      inline void setF7(bool value)
      {
        if(value)
          snd |= SL_F7;
        else
          snd &= ~SL_F7;
      }

      inline bool f8() const
      {
        return snd & SL_F8;
      }

      inline void setF8(bool value)
      {
        if(value)
          snd |= SL_F8;
        else
          snd &= ~SL_F8;
      }
    };
    static_assert(sizeof(LocoSnd) == 4);


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

      InputRep() :
        Message{OPC_INPUT_REP}
      {
      }

      inline uint16_t address() const
      {
        return (in1 & 0x7F) | (static_cast<uint16_t>(in2 & 0x0F) << 7);
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



    struct RequestSlotData : Message
    {
      uint8_t slot;
      uint8_t data2;
      uint8_t checksum;

      RequestSlotData(uint8_t _slot) : 
        Message(OPC_RQ_SL_DATA),
        slot{_slot},
        data2{0}
      {
        checksum = calcChecksum(*this);
      }
    };
    static_assert(sizeof(RequestSlotData) == 4);


    // d0 00 42 20 13 5e

    struct MultiSense : Message
    {
      uint8_t data1;
      uint8_t data2;
      uint8_t data3;
      uint8_t data4;
      uint8_t checksum;

      MultiSense() :
        Message(OPC_MULTI_SENSE)
      {
      }

      bool isTransponder() const
      {
        return
          ((data1 & MULTI_SENSE_TYPE_MASK) == MULTI_SENSE_TYPE_TRANSPONDER_GONE) ||
          ((data1 & MULTI_SENSE_TYPE_MASK) == MULTI_SENSE_TYPE_TRANSPONDER_PRESENT);
      }
    };
    static_assert(sizeof(MultiSense) == 6);

    struct MultiSenseTransponder : MultiSense
    {
      bool isPresent() const
      {
        return (data1 & MULTI_SENSE_TYPE_MASK) == MULTI_SENSE_TYPE_TRANSPONDER_PRESENT;
      }

      uint16_t sensorAddress() const
      {
        return (static_cast<uint16_t>(data1 & 0x1F) << 7) | (data2 & 0x7F);
      }

      uint16_t transponderAddress() const
      {
        if(isTransponderAddressLong())
          return (static_cast<uint16_t>(data3 & 0x7F) << 7) | (data4 & 0x7F);
        else
          return (data4 & 0x7F);
      }

      bool isTransponderAddressLong() const
      {
        return data3 != MULTI_SENSE_TRANSPONDER_ADDRESS_SHORT;
      }
    };
    static_assert(sizeof(MultiSenseTransponder) == 6);

    struct MultiSenseLong : Message
    {
      uint8_t len;
      uint8_t data1;
      uint8_t data2;
      uint8_t data3;
      uint8_t data4;
      uint8_t data5;
      uint8_t data6;
      uint8_t checksum;

      MultiSenseLong() :
        Message(OPC_MULTI_SENSE_LONG),
        len{9}
      {
      }

      bool isTransponder() const
      {
        return
          ((data1 & MULTI_SENSE_TYPE_MASK) == MULTI_SENSE_TYPE_TRANSPONDER_GONE) ||
          ((data1 & MULTI_SENSE_TYPE_MASK) == MULTI_SENSE_TYPE_TRANSPONDER_PRESENT);
      }
    };
    static_assert(sizeof(MultiSenseLong) == 9);

    struct MultiSenseLongTransponder : MultiSenseLong
    {
      bool isPresent() const
      {
        return (data1 & MULTI_SENSE_TYPE_MASK) == MULTI_SENSE_TYPE_TRANSPONDER_PRESENT;
      }

      uint16_t sensorAddress() const
      {
        return (static_cast<uint16_t>(data1 & 0x1F) << 7) | (data2 & 0x7F);
      }

      uint16_t transponderAddress() const
      {
        if(isTransponderAddressLong())
          return (static_cast<uint16_t>(data3 & 0x7F) << 7) | (data4 & 0x7F);
        else
          return (data4 & 0x7F);
      }

      bool isTransponderAddressLong() const
      {
        return data3 != MULTI_SENSE_TRANSPONDER_ADDRESS_SHORT;
      }

      Direction transponderDirection() const
      {
        return (data5 & 0x40) ? Direction::Forward : Direction::Reverse;
      }
    };
    static_assert(sizeof(MultiSenseLongTransponder) == 9);

    // OPC_SL_RD_DATA [E7 0E 1F 13 6F 01 30 07 08 19 00 00 00 52]
    struct SlotReadData : Message
    {
      uint8_t len;
      uint8_t slot;
      uint8_t stat;
      uint8_t adr;
      uint8_t spd;
      uint8_t dirf;
      uint8_t trk;
      uint8_t ss2;
      uint8_t adr2;
      uint8_t snd;
      uint8_t id1;
      uint8_t id2;
      uint8_t checksum;

      SlotReadData() :
        Message(OPC_SL_RD_DATA),
        len{14}
      {
      }

      bool isBusy() const
      {
        return stat & SL_BUSY;
      }

      bool isActive() const
      {
        return stat & SL_ACTIVE;
      }

      uint16_t address() const
      {
        return (static_cast<uint16_t>(adr2) << 7) | adr;
      }

      bool isEmergencyStop() const
      {
        return spd == 0x01;
      }

      uint8_t speed() const
      {
        return spd > 1 ? spd - 1 : 0;
      }

      inline Direction direction() const
      {
        return (dirf & SL_DIR) ? Direction::Forward : Direction::Reverse;
      }

      inline void setDirection(Direction value)
      {
        if(value == Direction::Forward)
          dirf |= SL_DIR;
        else
          dirf &= ~SL_DIR;
      }

      inline bool f0() const
      {
        return dirf & SL_F0;
      }

      inline void setF0(bool value)
      {
        if(value)
          dirf |= SL_F0;
        else
          dirf &= ~SL_F0;
      }

      inline bool f1() const
      {
        return dirf & SL_F1;
      }

      inline void setF1(bool value)
      {
        if(value)
          dirf |= SL_F1;
        else
          dirf &= ~SL_F1;
      }

      inline bool f2() const
      {
        return dirf & SL_F2;
      }

      inline void setF2(bool value)
      {
        if(value)
          dirf |= SL_F2;
        else
          dirf &= ~SL_F2;
      }

      inline bool f3() const
      {
        return dirf & SL_F3;
      }

      inline void setF3(bool value)
      {
        if(value)
          dirf |= SL_F3;
        else
          dirf &= ~SL_F3;
      }

      inline bool f4() const
      {
        return dirf & SL_F4;
      }

      inline void setF4(bool value)
      {
        if(value)
          dirf |= SL_F4;
        else
          dirf &= ~SL_F4;
      }

      inline bool f5() const
      {
        return snd & SL_F5;
      }

      inline void setF5(bool value)
      {
        if(value)
          snd |= SL_F5;
        else
          snd &= ~SL_F5;
      }

      inline bool f6() const
      {
        return snd & SL_F6;
      }

      inline void setF6(bool value)
      {
        if(value)
          snd |= SL_F6;
        else
          snd &= ~SL_F6;
      }

      inline bool f7() const
      {
        return snd & SL_F7;
      }

      inline void setF7(bool value)
      {
        if(value)
          snd |= SL_F7;
        else
          snd &= ~SL_F7;
      }

      inline bool f8() const
      {
        return snd & SL_F8;
      }

      inline void setF8(bool value)
      {
        if(value)
          snd |= SL_F8;
        else
          snd &= ~SL_F8;
      }
    };
    static_assert(sizeof(SlotReadData) == 14);

  protected:
    class Slots
    {
      private:
        std::unordered_map<uint16_t, uint8_t> m_addressToSlot;
        std::unordered_map<uint8_t, uint16_t> m_slotToAddress;

      public:
        uint8_t getSlot(uint16_t address) const
        {
          auto it = m_addressToSlot.find(address);
          return it != m_addressToSlot.end() ? it->second : SLOT_UNKNOWN;
        }

        uint16_t getAddress(uint8_t slot) const
        {
          auto it = m_slotToAddress.find(slot);
          return it != m_slotToAddress.end() ? it->second : 0;
        }

        void set(uint16_t address, uint8_t slot)
        {
          m_addressToSlot[address] = slot;
          m_slotToAddress[slot] = address;
        }

        void clear()
        {
          m_addressToSlot.clear();
          m_slotToAddress.clear();
        }
    };

    std::function<bool(const Message&)> m_send;
    std::atomic_bool m_debugLog;
    Slots m_slots;
    std::unordered_map<uint16_t, std::vector<std::byte>> m_slotRequests;
    uint8_t m_queryLocoSlots;
    std::unordered_map<uint16_t, std::shared_ptr<LocoNetInput>> m_inputs;

    std::shared_ptr<Hardware::Decoder> getDecoder(uint8_t slot, bool request = true);

  public://protected:
    bool isInputAddressAvailable(uint16_t address);
    bool addInput(const std::shared_ptr<LocoNetInput>& input);
    void removeInput(const std::shared_ptr<LocoNetInput>& input);

  public:
    CLASS_ID("protocol.loconet")

    Property<LocoNetCommandStation> commandStation;
    Property<bool> debugLog;

    LocoNet(Object& _parent, const std::string& parentPropertyName, std::function<bool(const Message&)> send);

    bool send(const Message& message);
    void send(uint16_t address, SlotMessage& message);
    void receive(const Message& message);

    void decoderChanged(const Hardware::Decoder& decoder, Hardware::DecoderChangeFlags changes, uint32_t functionNumber);

    void queryLocoSlots();
};

}

#endif



