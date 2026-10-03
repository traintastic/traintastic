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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLIN6023_CONFIG_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLIN6023_CONFIG_HPP

#include <cstdint>

namespace Marklin6023 {

struct Config
{
  unsigned int s88amount   = 1;   ///< number of S88 modules (max 4 for 6023/6223)
  unsigned int s88interval = 400; ///< milliseconds between S88 poll cycles
  unsigned int redundancy  = 0;   ///< extra retransmit count (0 = send once)
  bool         debugLogRXTX = false; ///< log every TX/RX line to the debug log

  bool         commandQueue    = false; ///< pace outgoing commands through a FIFO queue
  unsigned int commandInterval = 100;   ///< milliseconds between queued command sends (10..500)
  bool         ignoreWarnings  = false; ///< disable the auto power-off on crash/overflow (still logged)
  bool         crashDetection  = false; ///< monitor CTS; stop the world if low > 10 s
  bool         waitForCts      = false; ///< queue only sends while CTS is asserted (needs crashDetection)
};

} // namespace Marklin6023

#endif
