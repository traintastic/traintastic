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

#include "rclinksimulator.hpp"
#include <cstring>
#include "../rclinkmessages.hpp"
#include "../../../../utils/inrange.hpp"

namespace RCLink {

namespace {

template<typename T>
requires(std::is_base_of_v<Message, T>)
inline std::span<const uint8_t> toSpan(const T& message)
{
  return {reinterpret_cast<const uint8_t*>(&message), sizeof(message)};
}

}

Simulator::Simulator()
{
  std::fill(m_inputs.begin(), m_inputs.end(), false);
}

void Simulator::receive(std::span<const uint8_t> message)
{
  if(InitializeAll::check(message))
  {
    for(uint8_t address = inputAddressMin; address <= inputAddressMax; ++address)
    {
      Detector detector(address, m_inputs[address - inputAddressMin] ? Detector::railcomNoAddress : 0, false);
      send(toSpan(detector));
    }
  }
  else if(QueryInfo::check(message))
  {
    Info info(1, 10, 10);
    send(toSpan(info));
  }
}

void Simulator::detectorEvent(uint8_t address, bool occupied)
{
  if(inRange<uint8_t>(address, inputAddressRange))
  {
    m_inputs[address - inputAddressMin] = occupied;
    Detector detector(address++, occupied ? Detector::railcomNoAddress : 0, false);
    send(toSpan(detector));
  }
}

void Simulator::detectorEventToggle(uint8_t address)
{
  if(inRange<uint8_t>(address, inputAddressRange))
  {
    detectorEvent(address, !m_inputs[address - inputAddressMin]);
  }
}

void Simulator::send(std::span<const uint8_t> message)
{
  assert(onSend);
  onSend(message);
}

}

