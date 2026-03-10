/**
 * server/src/hardware/protocol/Marklin6050Interface/config.hpp
 *
 * Copyright (C) 2025
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLIN6050_CONFIG_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLIN6050_CONFIG_HPP

#include <cstdint>
#include "protocol.hpp"

namespace Marklin6050 {

struct Config
{
    ProtocolMode protocolMode = ProtocolMode::Binary;
    uint16_t centralUnitVersion = 6020;
    bool analog = false;
    unsigned int s88amount = 1;
    unsigned int s88interval = 400;
    unsigned int turnouttime = 200;
    unsigned int redundancy = 0;
    bool extensions = false;
};

} // namespace Marklin6050

#endif
