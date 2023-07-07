/**
 * server/src/hardware/protocol/dccplusplus/settings.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023 Reinder Feenstra
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

namespace DCCPlusPlus {

Settings::Settings(Object& _parent, std::string_view parentPropertyName)
  : SubObject(_parent, parentPropertyName)
  , useEx{this, "use_ex", true, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , speedSteps{this, "speed_steps", 128, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , startupDelay{this, "startup_delay", startupDelayDefault, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , debugLogRXTX{this, "debug_log", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  Attributes::addEnabled(useEx, false); // disable for now, only ex is currently supported
  m_interfaceItems.add(useEx);

  Attributes::addDisplayName(speedSteps, DisplayName::Hardware::speedSteps);
  Attributes::addEnabled(speedSteps, false);
  Attributes::addValues(speedSteps, speedStepValues);
  m_interfaceItems.add(speedSteps);

  Attributes::addMinMax(startupDelay, startupDelayMin, startupDelayMax);
  m_interfaceItems.add(startupDelay);

  Attributes::addDisplayName(debugLogRXTX, DisplayName::Hardware::debugLogRXTX);
  m_interfaceItems.add(debugLogRXTX);
}

Config Settings::config() const
{
  Config config;
  config.useEx = useEx;
  config.speedSteps = speedSteps;
  config.startupDelay = startupDelay;
  config.debugLogRXTX = debugLogRXTX;
  return config;
}

}
