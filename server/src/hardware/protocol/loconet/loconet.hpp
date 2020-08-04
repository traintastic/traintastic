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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_LOCONET_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_LOCONET_HPP

#include <vector>
#include "../../../core/subobject.hpp"
#include "../../../core/property.hpp"
#include <traintastic/enum/direction.hpp>
#include "../../../enum/loconetcommandstation.hpp"
//#include <cstdint>
//#include <cassert>
#include "../../../hardware/decoder/decoderchangeflags.hpp"
#include "messages.hpp"

class CommandStation;
class Decoder;
class LocoNetInput;

namespace LocoNet {

class LocoNet : public SubObject
{
  //friend class LocoNetInput;

  protected:
    static constexpr bool isLongAddress(uint16_t address)
    {
      return address > 127;
    }

  public:

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

    CommandStation* const m_commandStation; // valid if parent is command station, else nullptr
    std::function<bool(const Message&)> m_send;
    std::atomic_bool m_debugLog;
    Slots m_slots;
    std::unordered_map<uint16_t, std::vector<std::byte>> m_slotRequests;
    uint8_t m_queryLocoSlots;
    std::unordered_map<uint16_t, std::shared_ptr<LocoNetInput>> m_inputs;

    std::shared_ptr<Decoder> getDecoder(uint8_t slot, bool request = true);

    void send(uint16_t address, Message& message, uint8_t& slot);
    template<typename T>
    inline void send(uint16_t address, T& message)
    {
      send(address, message, message.slot);
    }

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
    void receive(const Message& message);

    void emergencyStopChanged(bool value);
    void trackVoltageOffChanged(bool value);
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber);

    void queryLocoSlots();
};

}

#endif
