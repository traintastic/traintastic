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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLIN6023_PROTOCOL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLIN6023_PROTOCOL_HPP

/**
 * Protocol constants for the Märklin 6023/6223 ASCII serial interface.
 *
 * All commands are ASCII strings terminated with CR (\r).
 * Responses are ASCII strings terminated with CR or LF.
 *
 * Command summary
 * ---------------
 *   G\r              – global go (power on)
 *   S\r              – global stop (power off)
 *   L <addr> S <spd> F <f0>\r  – loco speed + F0 (addr 10-40, spd 0-14)
 *   L <addr> D\r                – loco direction toggle
 *   M <addr> R\r                – accessory red  (CU handles solenoid off)
 *   M <addr> G\r                – accessory green
 *   C <contact>\r               – query S88 contact (1-based); returns "0" or "1"
 */
namespace Marklin6023 {

constexpr char CR = '\r';

} // namespace Marklin6023

#endif
