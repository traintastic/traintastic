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

#include "cbussettings.hpp"
#include "../../protocol/cbus/cbusconst.hpp"
#include "../../../core/attributes.hpp"
#include "../../../utils/displayname.hpp"
#include "../../../utils/unit.hpp"

CBUSSettings::CBUSSettings(Object& _parent, std::string_view parentPropertyName)
  : SubObject(_parent, parentPropertyName)
  , engineKeepAlive{this, "engine_keep_alive", engineKeepAliveDefault, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , dccAccessorySwitchTime{this, "dcc_accessory_switch_time", dccAccessorySwitchTimeDefault, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , shortEventNodeNumber{this, "short_event_node_number", 0, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , hubEnabled{this, "hub_enabled", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , hubLocalhostOnly{this, "hub_localhost_only", true, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , hubPort{this, "hub_port", CBUS::defaultTCPPort, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , debugLogRXTX{this, "debug_log_rx_tx", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  Attributes::addMinMax(engineKeepAlive, engineKeepAliveMin, engineKeepAliveMax);
  Attributes::addUnit(engineKeepAlive, Unit::seconds);
  m_interfaceItems.add(engineKeepAlive);

  Attributes::addMinMax(dccAccessorySwitchTime, dccAccessorySwitchTimeMin, dccAccessorySwitchTimeMax);
  Attributes::addStep(dccAccessorySwitchTime, dccAccessorySwitchTimeStep);
  Attributes::addUnit(dccAccessorySwitchTime, Unit::milliSeconds);
  m_interfaceItems.add(dccAccessorySwitchTime);

  m_interfaceItems.add(shortEventNodeNumber);

  m_interfaceItems.add(hubEnabled);

  m_interfaceItems.add(hubLocalhostOnly);

  m_interfaceItems.add(hubPort);

  Attributes::addDisplayName(debugLogRXTX, DisplayName::Hardware::debugLogRXTX);
  //Attributes::addGroup(debugLogRXTX, Group::debug);
  m_interfaceItems.add(debugLogRXTX);
}

CBUS::Config CBUSSettings::config() const
{
  return CBUS::Config{
    .engineKeepAlive = std::chrono::seconds(engineKeepAlive),
    .dccAccessorySwitchTime = std::chrono::milliseconds(dccAccessorySwitchTime),
    .shortEventNodeNumber = shortEventNodeNumber,
    .hubEnabled = hubEnabled,
    .hubLocalhostOnly = hubLocalhostOnly,
    .hubPort = hubPort,
    .debugLogRXTX = debugLogRXTX,
  };
}
