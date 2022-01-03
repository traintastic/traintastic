/**
 * server/src/hardware/protocol/z21/serversettings.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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

#include "serversettings.hpp"

namespace Z21 {

ServerSettings::ServerSettings(Object& _parent, const std::string& parentPropertyName)
  : Settings(_parent, parentPropertyName)
  , allowEmergencyStop{this, "allow_emergency_stop", true, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , allowTrackPowerOff{this, "allow_track_power_off", true, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , allowTrackPowerOnReleaseEmergencyStop{this, "allow_track_power_on_release_emergency_stop", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  m_interfaceItems.insertBefore(allowEmergencyStop, debugLogRXTX);
  m_interfaceItems.insertBefore(allowTrackPowerOff, debugLogRXTX);
  m_interfaceItems.insertBefore(allowTrackPowerOnReleaseEmergencyStop, debugLogRXTX);
}

ServerConfig ServerSettings::config() const
{
  ServerConfig config;

  getConfig(config);

  config.allowEmergencyStop = allowEmergencyStop;
  config.allowTrackPowerOff = allowTrackPowerOff;
  config.allowTrackPowerOnReleaseEmergencyStop = allowTrackPowerOnReleaseEmergencyStop;

  return config;
}

}
