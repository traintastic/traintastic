/**
 * server/src/hardware/protocol/marklincan/settings.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

namespace MarklinCAN {

Settings::Settings(Object& _parent, std::string_view parentPropertyName)
  : SubObject(_parent, parentPropertyName)
  , defaultSwitchTime{this, "default_switch_time", 0, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , debugLogRXTX{this, "debug_log_rx_tx", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , debugConfigStream{this, "debug_config_stream", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  Attributes::addMinMax<uint32_t>(defaultSwitchTime, 0, 163'000);
  //Attributes::addStep(defaultSwitchTime, 10);
  m_interfaceItems.add(defaultSwitchTime);

  Attributes::addDisplayName(debugLogRXTX, DisplayName::Hardware::debugLogRXTX);
  m_interfaceItems.add(debugLogRXTX);

  m_interfaceItems.add(debugConfigStream);
}

Config Settings::config() const
{
  Config config;

  config.defaultSwitchTime = defaultSwitchTime;
  config.debugLogRXTX = debugLogRXTX;
  config.debugConfigStream = debugConfigStream;

  return config;
}

}
