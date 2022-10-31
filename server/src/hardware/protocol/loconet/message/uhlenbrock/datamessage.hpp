/**
 * server/src/hardware/protocol/loconet/message/uhlenbrock/datamessage.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_MESSAGE_UHLENBROCK_DATAMESSAGE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_MESSAGE_UHLENBROCK_DATAMESSAGE_HPP

#include "../message.hpp"

namespace LocoNet::Uhlenbrock {

struct DataMessage : Message
{
  static constexpr uint8_t dataLen = 12;

  uint8_t len = 0x0F;
  uint8_t data[dataLen] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  uint8_t checksum = 0;

  DataMessage(OpCode opc)
    : Message(opc)
  {
  }

  template<uint8_t N>
  uint8_t getData() const
  {
    static_assert(N >= 5 && N <= 10);
    return (data[4] & static_cast<uint8_t>(1 << (N - 5)) ? 0x80 : 0x00) | data[N];
  }

  template<uint8_t N>
  void setData(uint8_t value)
  {
    static_assert(N >= 5 && N <= 10);
    data[N] = value & 0x7F;
    if(value & 0x80)
      data[4] |= static_cast<uint8_t>(1 << (N - 5));
    else
      data[4] &= ~static_cast<uint8_t>(1 << (N - 5));
  }
};
static_assert(sizeof(DataMessage) == 15);

template<class T, class Tbase>
inline bool checkMagicData(const Tbase& message)
{
  static_assert(std::is_base_of_v<Tbase, T>);
  static_assert(sizeof(T) == sizeof(Tbase));
  static_assert(Tbase::dataLen == T::magicData.size());
  static_assert(Tbase::dataLen == T::magicMask.size());
  for(uint8_t i = 0; i < Tbase::dataLen; i++)
    if((message.data[i] & T::magicMask[i]) != T::magicData[i])
      return false;
  return true;
}

template<class T>
void setMagicData(T& message)
{
  static_assert(std::is_base_of_v<DataMessage, T>);
  static_assert(T::dataLen == T::magicData.size());
  static_assert(T::dataLen == T::magicMask.size());
  for(uint8_t i = 0; i < T::dataLen; i++)
    message.data[i] = (message.data[i] & ~T::magicMask[i]) | T::magicData[i];
}

struct ImmPacketDataMessage : DataMessage
{
  inline static bool check(const Message& message)
  {
    return
      (message.opCode == OPC_IMM_PACKET) &&
      (message.size() == sizeof(ImmPacketDataMessage));
  }

  ImmPacketDataMessage()
    : DataMessage(OPC_IMM_PACKET)
  {
  }
};
static_assert(sizeof(ImmPacketDataMessage) == 15);

struct PeerXferDataMessage : DataMessage
{
  inline static bool check(const Message& message)
  {
    return
      (message.opCode == OPC_PEER_XFER) &&
      (message.size() == sizeof(PeerXferDataMessage));
  }

  PeerXferDataMessage()
    : DataMessage(OPC_PEER_XFER)
  {
  }
};
static_assert(sizeof(PeerXferDataMessage) == 15);

}

#endif
