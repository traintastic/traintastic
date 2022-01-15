/**
 * server/src/hardware/protocol/loconet/settings.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022 Reinder Feenstra
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

namespace LocoNet {

Settings::Settings(Object& _parent, std::string_view parentPropertyName)
  : SubObject(_parent, parentPropertyName)
  , commandStation{this, "command_station", LocoNetCommandStation::Custom, PropertyFlags::ReadWrite | PropertyFlags::Store,
      [](LocoNetCommandStation value)
      {
        switch(value)
        {
          case LocoNetCommandStation::Custom:
            break;

          case LocoNetCommandStation::DigikeijsDR5000:
            break;

          case LocoNetCommandStation::UhlenbrockIntellibox:
            break;
        }
      }}
  , fastClockSyncEnabled{this, "fast_clock_sync_enabled", false, PropertyFlags::ReadWrite | PropertyFlags::Store,
      [this](bool value)
      {
        Attributes::setEnabled(fastClockSyncInterval, value);
      }}
  , fastClockSyncInterval{this, "fast_clock_sync_interval", 60, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , debugLogInput{this, "debug_log_input", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , debugLogOutput{this, "debug_log_output", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , debugLogRXTX{this, "debug_log_rx_tx", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  Attributes::addDisplayName(commandStation, DisplayName::Hardware::commandStation);
  Attributes::addValues(commandStation, LocoNetCommandStationValues);
  m_interfaceItems.add(commandStation);

  //Attributes::addGroup(fastClockSyncEnabled, Group::fastClockSync);
  m_interfaceItems.add(fastClockSyncEnabled);

  Attributes::addEnabled(fastClockSyncInterval, fastClockSyncEnabled);
  //Attributes::addGroup(fastClockSyncInterval, Group::fastClockSync);
  m_interfaceItems.add(fastClockSyncInterval);

  Attributes::addDisplayName(debugLogInput, DisplayName::Hardware::debugLogInput);
  //Attributes::addGroup(debugLogInput, Group::debug);
  m_interfaceItems.add(debugLogInput);

  Attributes::addDisplayName(debugLogOutput, DisplayName::Hardware::debugLogOutput);
  //Attributes::addGroup(debugLogOuput, Group::debug);
  m_interfaceItems.add(debugLogOutput);

  Attributes::addDisplayName(debugLogRXTX, DisplayName::Hardware::debugLogRXTX);
  //Attributes::addGroup(debugLogRXTX, Group::debug);
  m_interfaceItems.add(debugLogRXTX);
}

Config Settings::config() const
{
  Config config;

  config.fastClockSyncEnabled = fastClockSyncEnabled;
  config.fastClockSyncInterval = fastClockSyncInterval;

  config.debugLogInput = debugLogInput;
  config.debugLogOutput = debugLogOutput;
  config.debugLogRXTX = debugLogRXTX;

  return config;
}

void Settings::loaded()
{
  SubObject::loaded();

  Attributes::setEnabled(fastClockSyncInterval, fastClockSyncEnabled);
}

}
