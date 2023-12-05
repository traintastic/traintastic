/**
 * server/src/hardware/protocol/traintasticdiy/settings.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2023 Reinder Feenstra
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

#include "settings.hpp"
#include "../../../core/attributes.hpp"
#include "../../../utils/displayname.hpp"

namespace TraintasticDIY {

Settings::Settings(Object& _parent, std::string_view parentPropertyName)
  : SubObject(_parent, parentPropertyName)
  , startupDelay{this, "startup_delay", startupDelayDefault, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , heartbeatTimeout{this, "heartbeat_timeout", heartbeatTimeoutDefault, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , debugLogRXTX{this, "debug_log_rx_tx", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , debugLogHeartbeat{this, "debug_log_heartbeat", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  Attributes::addMinMax(startupDelay, startupDelayMin, startupDelayMax);
  m_interfaceItems.add(startupDelay);

  Attributes::addMinMax(heartbeatTimeout, heartbeatTimeoutMin, heartbeatTimeoutMax);
  m_interfaceItems.add(heartbeatTimeout);

  Attributes::addDisplayName(debugLogRXTX, DisplayName::Hardware::debugLogRXTX);
  m_interfaceItems.add(debugLogRXTX);

  m_interfaceItems.add(debugLogHeartbeat);
}

Config Settings::config() const
{
  Config config;

  config.startupDelay = std::chrono::milliseconds(startupDelay);
  config.heartbeatTimeout = std::chrono::milliseconds(heartbeatTimeout);

  config.debugLogRXTX = debugLogRXTX;
  config.debugLogHeartbeat = debugLogHeartbeat;

  return config;
}

}
