/**
 * server/src/hardware/protocol/traintasticcs/settings.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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
#include "messages.hpp"
#include "../../../core/attributes.hpp"
#include "../../../utils/displayname.hpp"

namespace TraintasticCS {

Settings::Settings(Object& _parent, std::string_view parentPropertyName)
  : SubObject(_parent, parentPropertyName)
  , s88Enabled{this, "s88_enabled", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , s88ModuleCount{this, "s88_module_count", 2, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , s88ClockFrequency{this, "s88_clock_frequency", 10, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , xpressnetEnabled{this, "xpressnet_enabled", true, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , debugLogRXTX{this, "debug_log_rx_tx", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , debugLogPing{this, "debug_log_ping", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  Attributes::addEnabled(s88Enabled, false);
  //Attributes::addGroup(s88Enabled, Group::s88);
  m_interfaceItems.add(s88Enabled);

  Attributes::addEnabled(s88ModuleCount, false);
  //Attributes::addGroup(s88ModuleCount, Group::s88);
  Attributes::addMinMax(s88ModuleCount, Config::S88::moduleCountMin, Config::S88::moduleCountMax);
  m_interfaceItems.add(s88ModuleCount);

  Attributes::addEnabled(s88ClockFrequency, false);
  //Attributes::addGroup(s88ClockFrequency, Group::s88);
  Attributes::addMinMax(s88ClockFrequency, Config::S88::clockFrequencyMin, Config::S88::clockFrequencyMax);
  Attributes::addUnit(s88ClockFrequency, "kHz");
  m_interfaceItems.add(s88ClockFrequency);

  Attributes::addEnabled(xpressnetEnabled, false);
  //Attributes::addGroup(xpressnetEnabled, Group::xpressnet);
  m_interfaceItems.add(xpressnetEnabled);

  Attributes::addEnabled(debugLogRXTX, false);
  Attributes::addDisplayName(debugLogRXTX, DisplayName::Hardware::debugLogRXTX);
  //Attributes::addGroup(debugLogRXTX, Group::debug);
  m_interfaceItems.add(debugLogRXTX);

  Attributes::addEnabled(debugLogPing, false);
  //Attributes::addGroup(debugLogPing, Group::debug);
  m_interfaceItems.add(debugLogPing);
}

Config Settings::config() const
{
  Config config;

  config.s88.enabled = s88Enabled;
  config.s88.moduleCount = s88ModuleCount;
  config.s88.clockFrequency = s88ClockFrequency;

  config.xpressnet.enabled = xpressnetEnabled;

  config.debugLogRXTX = debugLogRXTX;
  config.debugLogPing = debugLogPing;

  return config;
}

void Settings::updateEnabled(const bool worldEdit, const bool interfaceOnline)
{
  Attributes::setEnabled(
    {
      s88Enabled,
      s88ModuleCount,
      s88ClockFrequency,
      xpressnetEnabled
    }, worldEdit && !interfaceOnline);
  Attributes::setEnabled(
    {
      debugLogRXTX,
      debugLogPing
    }, worldEdit);
}

}
