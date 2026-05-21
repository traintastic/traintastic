/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Kamil Kasprzak
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLIN6050_CONFIG_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLIN6050_CONFIG_HPP

#include <cstdint>

namespace Marklin6050 {

struct Config
{
  uint16_t     centralUnitVersion = 6020;
  bool         analog             = false;
  unsigned int s88amount          = 1;
  unsigned int s88interval        = 400;  ///< milliseconds between S88 polls
  unsigned int turnouttime        = 200;  ///< milliseconds solenoid on-time
  unsigned int redundancy         = 0;    ///< extra retransmit count (0 = send once)
  bool         extensions         = false; ///< enable extension event polling
  bool         debugLogRXTX       = false; ///< log every TX/RX byte to debug log
};

} // namespace Marklin6050

#endif
