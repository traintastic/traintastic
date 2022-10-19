/**
 * server/src/hardware/protocol/loconet/messages/uhlenbrock/lissy.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_MESSAGES_UHLENBROCK_LISSY_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_MESSAGES_UHLENBROCK_LISSY_HPP

#include "../message.hpp"
#include <traintastic/enum/direction.hpp>

namespace LocoNet::Uhlenbrock {

/**
 * \addtogroup loconet_messages_uhlenbrock
 * \{
 *   \defgroup loconet_messages_uhlenbrock_lissy LISSY
 *   \{
 */

struct Lissy : Message
{
  enum class Type
  {
    AddressCategoryDirection,
    Speed,
  };

  uint8_t len = 8;
  uint8_t data1 = 0;
  uint8_t sensorAddressHigh = 0;
  uint8_t sensorAddressLow = 0;
  uint8_t data4 = 0;
  uint8_t data5 = 0;
  uint8_t checksum = 0;

  Lissy()
    : Message(OPC_E4)
  {
  }

  Type type() const
  {
    return (sensorAddressHigh & 0x60) == 0x20 ? Type::Speed : Type::AddressCategoryDirection;
  }

  uint16_t sensorAddress() const
  {
    return static_cast<uint16_t>(sensorAddressHigh & 0x1F) << 7 | sensorAddressLow;
  }

};
static_assert(sizeof(Lissy) == 8);

struct LissyAddressCategoryDirection : Lissy
{
  uint16_t decoderAddress() const
  {
    return static_cast<uint16_t>(data4) << 7 | data5;
  }

  /**
   * \brief Lissy reported category
   * \return Category number: 1...4
   */
  uint8_t category() const
  {
    return (data1 & 0x03) + 1;
  }

  Direction direction() const
  {
    switch(sensorAddressHigh & 0x60)
    {
      case 0x00:
        return Direction::Unknown;

      case 0x40:
        return Direction::Forward; // TODO: verify this

      case 0x60:
        return Direction::Reverse; // TODO: verify this
    }
    assert(false);
    return Direction::Unknown;
  }
};
static_assert(sizeof(LissyAddressCategoryDirection) == 8);

struct LissySpeed : Lissy
{
  uint16_t speed() const
  {
    return static_cast<uint16_t>(data4) << 7 | data5; // TODO: verify this, no clue yet...
  }
};
static_assert(sizeof(LissySpeed) == 8);

/**
 *   \}
 * \}
 */

}

#endif
