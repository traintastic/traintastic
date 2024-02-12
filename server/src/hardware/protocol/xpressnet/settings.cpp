/**
 * server/src/hardware/protocol/xpressnet/settings.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021,2024 Reinder Feenstra
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

namespace XpressNet {

Settings::Settings(Object& _parent, std::string_view parentPropertyName)
  : SubObject(_parent, parentPropertyName)
  , commandStation{this, "command_station", XpressNetCommandStation::Custom, PropertyFlags::ReadWrite | PropertyFlags::Store, std::bind(&Settings::commandStationChanged, this, std::placeholders::_1)}
  , useEmergencyStopLocomotiveCommand{this, "use_emergency_stop_locomotive_command", false, PropertyFlags::ReadWrite | PropertyFlags::Store, std::bind(&Settings::setCommandStationCustom, this)}
  , useRocoAccessoryAddressing{this, "use_roco_accessory_addressing", false, PropertyFlags::ReadWrite | PropertyFlags::Store, std::bind(&Settings::setCommandStationCustom, this)}
  , useRocoF13F20Command{this, "use_roco_f13_f20_command", false, PropertyFlags::ReadWrite | PropertyFlags::Store, std::bind(&Settings::setCommandStationCustom, this)}
  , debugLogInput{this, "debug_log_input", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , debugLogRXTX{this, "debug_log_rx_tx", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  Attributes::addDisplayName(commandStation, DisplayName::Hardware::commandStation);
  Attributes::addValues(commandStation, XpressNetCommandStationValues);
  m_interfaceItems.add(commandStation);

  m_interfaceItems.add(useEmergencyStopLocomotiveCommand);

  m_interfaceItems.add(useRocoAccessoryAddressing);

  m_interfaceItems.add(useRocoF13F20Command);

  Attributes::addDisplayName(debugLogInput, DisplayName::Hardware::debugLogInput);
  m_interfaceItems.add(debugLogInput);

  Attributes::addDisplayName(debugLogRXTX, DisplayName::Hardware::debugLogRXTX);
  m_interfaceItems.add(debugLogRXTX);
}

Config Settings::config() const
{
  Config config;

  config.useEmergencyStopLocomotiveCommand = useEmergencyStopLocomotiveCommand;
  config.useRocoAccessoryAddressing = useRocoAccessoryAddressing;
  config.useRocoF13F20Command = useRocoF13F20Command;

  config.debugLogInput = debugLogInput;
  config.debugLogRXTX = debugLogRXTX;

  return config;
}

void Settings::loaded()
{
  SubObject::loaded();

  commandStationChanged(commandStation);
}

void Settings::commandStationChanged(XpressNetCommandStation value)
{
  switch(value)
  {
    case XpressNetCommandStation::Custom:
      break;

    case XpressNetCommandStation::Roco10764:
      useEmergencyStopLocomotiveCommand.setValueInternal(false);
      useRocoF13F20Command.setValueInternal(true);
      break;

    case XpressNetCommandStation::DigikeijsDR5000:
      useEmergencyStopLocomotiveCommand.setValueInternal(false);
      useRocoF13F20Command.setValueInternal(false);
      break;
  }
}

void Settings::setCommandStationCustom()
{
  commandStation.setValueInternal(XpressNetCommandStation::Custom);
}

}
