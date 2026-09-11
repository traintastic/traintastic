/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_MESSAGES_CBUSNODEMANAGEMENTMESSAGES_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_MESSAGES_CBUSNODEMANAGEMENTMESSAGES_HPP

#include "cbusmessage.hpp"
#include "../../../../utils/bit.hpp"
#include "../../../../utils/byte.hpp"

namespace CBUS {

struct PresenceOfNode : NodeMessage
{
  static constexpr uint8_t isConsumerNodeBit = 0;
  static constexpr uint8_t isProducerNodeBit = 1;
  static constexpr uint8_t flimModeBit = 2;
  static constexpr uint8_t isBootloaderCompatibleBit = 3;
  static constexpr uint8_t consumesOwnProducedEventsBit = 4;
  static constexpr uint8_t inLearnModeBit = 5;
  static constexpr uint8_t supportsServiceDiscoveryBit = 6;

  uint8_t manufacturerId;
  uint8_t moduleId;
  uint8_t flags;

  PresenceOfNode(uint16_t nodeNumber_, uint8_t manufacturerId_, uint8_t moduleId_,
        bool isConsumerNode_, bool isProducerNode_, bool flimMode_, bool isBootloaderCompatible_,
        bool consumesOwnProducedEvents_, bool inLearnMode_, bool supportsServiceDiscovery_)
    : NodeMessage(OpCode::PNN, nodeNumber_)
    , manufacturerId{manufacturerId_}
    , moduleId{moduleId_}
    , flags{0}
  {
    setBit<isConsumerNodeBit>(flags, isConsumerNode_);
    setBit<isProducerNodeBit>(flags, isProducerNode_);
    setBit<flimModeBit>(flags, flimMode_);
    setBit<isBootloaderCompatibleBit>(flags, isBootloaderCompatible_);
    setBit<consumesOwnProducedEventsBit>(flags, consumesOwnProducedEvents_);
    setBit<inLearnModeBit>(flags, inLearnMode_);
    setBit<supportsServiceDiscoveryBit>(flags, supportsServiceDiscovery_);
  }

  bool isConsumerNode() const
  {
    return getBit<isConsumerNodeBit>(flags);
  }

  bool isProducerNode() const
  {
    return getBit<isProducerNodeBit>(flags);
  }

  bool flimMode() const
  {
    return getBit<flimModeBit>(flags);
  }

  bool isBootloaderCompatible() const
  {
    return getBit<isBootloaderCompatibleBit>(flags);
  }

  bool consumesOwnProducedEvents() const
  {
    return getBit<consumesOwnProducedEventsBit>(flags);
  }

  bool inLearnMode() const
  {
    return getBit<inLearnModeBit>(flags);
  }

  bool supportsServiceDiscovery() const
  {
    return getBit<supportsServiceDiscoveryBit>(flags);
  }
};

}

#endif
