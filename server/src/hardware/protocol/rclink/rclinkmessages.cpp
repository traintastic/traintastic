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

#include "rclinkmessages.hpp"
#include "../../../compat/stdformat.hpp"
#include "../../../utils/tohex.hpp"

namespace RCLink {

std::string toString(std::span<const uint8_t> message)
{
  std::string s;

  if(InitializeAll::check(message))
  {
    s = "InitializeAll";
  }
  else if(DisableDataTraffic::check(message))
  {
    s = "DisableDataTraffic";
  }
  else if(QueryDetector::check(message))
  {
    const auto& msg = *reinterpret_cast<const QueryDetector*>(message.data());
    s = std::format("QueryDetector address={}", msg.address());
  }
  else if(QueryInfo::check(message))
  {
    s = "QueryInfo";
  }
  else if(ProgramDetector::check(message))
  {
    const auto& msg = *reinterpret_cast<const ProgramDetector*>(message.data());
    s = std::format("ProgramDetector address={}", msg.address());
  }
  else if(RequestDiagnosticData::check(message))
  {
    const auto& msg = *reinterpret_cast<const RequestDiagnosticData*>(message.data());
    s = std::format("RequestDiagnosticData address={}", msg.address());
  }
  else if(ScopeMode::check(message))
  {
    s = "ScopeMode";
  }
  else if(SystemOff::check(message))
  {
    s = "SystemOff";
  }
  else if(Detector::check(message))
  {
    const auto& msg = *reinterpret_cast<const Detector*>(message.data());
    s = std::format("Detector address={}", msg.address);;
    if(msg.hasRailcomAddress())
    {
      s.append(std::format(" railcomAddress={} orientation={}", msg.railcomAddress(), msg.orientation() ? "1" : "0"));
    }
    s.append(msg.occupied() ? " occupied" : " free");
  }
  else if(Info::check(message))
  {
    const auto& msg = *reinterpret_cast<const Info*>(message.data());
    s = std::format("Info serialNumber={} softwareVersion={}.{} hardwareVersion={}.{}",
      msg.serialNumber(),
      msg.softwareVersionMajor(),
      msg.softwareVersionMinor(),
      msg.hardwareVersionMajor(),
      msg.hardwareVersionMinor());
  }

  // raw data:
  s.append(" [").append(toHex(message.data(), message.size(), true)).append("]");

  return s;
}

}
