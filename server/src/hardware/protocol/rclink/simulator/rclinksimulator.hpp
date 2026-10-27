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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_RCLINK_SIMULATOR_RCLINKSIMULATOR_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_RCLINK_SIMULATOR_RCLINKSIMULATOR_HPP

#include <array>
#include <cstdint>
#include <functional>
#include <span>
#include "../rclinkconst.hpp"

namespace RCLink {

class Simulator
{
public:
  std::function<void(std::span<const uint8_t>)> onSend;

  Simulator();

  void receive(std::span<const uint8_t> message);

  void detectorEvent(uint8_t address, bool occupied);
  void detectorEventToggle(uint8_t address);

private:
  std::array<bool, inputAddressMax - inputAddressMin + 1> m_inputs;

  void send(std::span<const uint8_t> message);
};

}

#endif
