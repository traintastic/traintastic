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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DINAMO_SIMULATOR_DINAMOSIMULATOR_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DINAMO_SIMULATOR_DINAMOSIMULATOR_HPP

#include <array>
#include <cstdint>
#include <mutex>
#include <queue>
#include <span>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace Dinamo {

class Simulator
{
public:
  std::tuple<std::span<const uint8_t>, bool, bool> process(std::span<const uint8_t> message, bool hold, bool fault);

  void inputEvent(uint16_t address, bool value);
  void inputEventToggle(uint16_t address);

private:
  std::array<uint8_t, 43> m_response;
  bool m_hold = false;
  bool m_fault = true;

  std::recursive_mutex m_mutex;
  std::queue<std::vector<uint8_t>> m_queue; // requires m_mutex locked
  std::unordered_map<uint16_t, bool> m_inputs; // requires m_mutex locked

  template<typename T>
  void queue(const T& message);

  std::tuple<std::span<const uint8_t>, bool, bool> response();
  template<typename T>
  std::tuple<std::span<const uint8_t>, bool, bool> response(const T& message);
};

}

#endif
