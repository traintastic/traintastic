/**
 * server/src/hardware/protocol/selectrix/settings.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023,2025 Reinder Feenstra
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
#include "../../../world/getworld.hpp"

namespace {

constexpr auto pollIntervalRange = std::make_pair<uint16_t, uint16_t>(100, 1000); // ms

}

namespace Selectrix {

Settings::Settings(Object& _parent, std::string_view parentPropertyName)
  : SubObject(_parent, parentPropertyName)
  , useRautenhausCommandFormat{this, "use_rautenhaus_command_format", false, PropertyFlags::ReadWrite | PropertyFlags::Store,
      [this](bool /*value*/)
      {
        updateVisible();
      }}
  , trackPowerPollInterval{this, "track_power_poll_interval", 250, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , locomotivePollInterval{this, "locomotive_poll_interval", 500, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , feedbackPollInterval{this, "feedback_poll_interval", 100, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , debugLogRXTX{this, "debug_log_rx_tx", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  Attributes::addEnabled(useRautenhausCommandFormat, false);
  m_interfaceItems.add(useRautenhausCommandFormat);

  Attributes::addMinMax(trackPowerPollInterval, pollIntervalRange);
  Attributes::addUnit(trackPowerPollInterval, "ms");
  Attributes::addVisible(trackPowerPollInterval, true);
  m_interfaceItems.add(trackPowerPollInterval);

  Attributes::addMinMax(locomotivePollInterval, pollIntervalRange);
  Attributes::addUnit(locomotivePollInterval, "ms");
  Attributes::addVisible(locomotivePollInterval, true);
  m_interfaceItems.add(locomotivePollInterval);

  Attributes::addMinMax(feedbackPollInterval, pollIntervalRange);
  Attributes::addUnit(feedbackPollInterval, "ms");
  Attributes::addVisible(feedbackPollInterval, true);
  m_interfaceItems.add(feedbackPollInterval);

  Attributes::addDisplayName(debugLogRXTX, DisplayName::Hardware::debugLogRXTX);
  //Attributes::addGroup(debugLogRXTX, Group::debug);
  m_interfaceItems.add(debugLogRXTX);

  updateEnabled(contains(getWorld(this).state, WorldState::Edit), false);
}

Config Settings::config() const
{
  Config config;

  // useRautenhausCommandFormat is not in config as it is read only once when kernel starts
  config.trackPowerPollInterval = std::chrono::milliseconds(trackPowerPollInterval.value());
  config.locomotivePollInterval = std::chrono::milliseconds(locomotivePollInterval.value());
  config.feedbackPollInterval = std::chrono::milliseconds(feedbackPollInterval.value());
  config.debugLogRXTX = debugLogRXTX;

  return config;
}

void Settings::updateEnabled(bool worldEdit, bool interfaceOnline)
{
  const bool editable = worldEdit && !interfaceOnline;

  Attributes::setEnabled(useRautenhausCommandFormat, editable);
}

void Settings::updateVisible()
{
  Attributes::setVisible(trackPowerPollInterval, !useRautenhausCommandFormat);
  Attributes::setVisible(locomotivePollInterval, !useRautenhausCommandFormat);
  Attributes::setVisible(feedbackPollInterval, !useRautenhausCommandFormat);
}

void Settings::loaded()
{
  SubObject::loaded();
  updateVisible();
}

}
