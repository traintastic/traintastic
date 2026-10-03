/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Kamil Kasprzak
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
#include "../../../utils/unit.hpp"

namespace Marklin6050 {

// ---------------------------------------------------------------------------
// Attribute value arrays — defined here for stable span lifetimes
// ---------------------------------------------------------------------------

static constexpr std::array<uint16_t, 7> kCuVersionValues{
  6020, 6021, 6022, 6027, 6029, 6030, 6032
};
static constexpr std::array<std::string_view, 7> kCuVersionLabels{
  "6020", "6021", "6022", "6027", "6029", "6030", "6032"
};

static constexpr std::array<unsigned int, 15> kS88Intervals{
  50, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1500, 2000, 2500, 3000
};
static constexpr std::array<std::string_view, 15> kS88IntervalLabels{
  "50ms","100ms","200ms","300ms","400ms","500ms","600ms",
  "700ms","800ms","900ms","1s","1.5s","2s","2.5s","3s"
};

static constexpr std::array<unsigned int, 12> kTurnoutTimes{
  25, 50, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000
};
static constexpr std::array<std::string_view, 12> kTurnoutTimeLabels{
  "25ms","50ms","100ms","200ms","300ms","400ms",
  "500ms","600ms","700ms","800ms","900ms","1s"
};

static constexpr std::array<unsigned int, 4> kRedundancyOptions{0, 1, 2, 3};
static constexpr std::array<std::string_view, 4> kRedundancyLabels{
  "$marklin6050_settings:redundancy_off$", "x2", "x3", "x4"
};

// ---------------------------------------------------------------------------

Settings::Settings(Object& parent, std::string_view parentPropertyName)
  : SubObject{parent, parentPropertyName}
  , centralUnitVersion{this, "central_unit_version", 6020,
        PropertyFlags::ReadWrite | PropertyFlags::Store,
        [this](const uint16_t& value){ centralUnitVersionChanged(value); }}
  , analog{this, "analog", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , s88amount{this, "s88amount", 1, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , s88interval{this, "s88interval", 400, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , turnouttime{this, "turnouttime", 200, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , redundancy{this, "redundancy", 0, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , extensions{this, "extensions", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , debugLogRXTX{this, "debug_log_rx_tx", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , commandQueue{this, "command_queue", false, PropertyFlags::ReadWrite | PropertyFlags::Store,
      [this](bool value)
      {
        Attributes::setEnabled(commandInterval, value);
      }}
  , commandInterval{this, "command_interval", 100, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , ignoreWarnings{this, "ignore_warnings", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , crashDetection{this, "crash_detection", false, PropertyFlags::ReadWrite | PropertyFlags::Store,
      [this](bool value)
      {
        Attributes::setEnabled(waitForCts, value);
      }}
  , waitForCts{this, "wait_for_cts", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  // centralUnitVersion — disabled when online; toggled by updateEnabled
  Attributes::addCategory(centralUnitVersion, "category:marklin_6050");
  Attributes::addDisplayName(centralUnitVersion, DisplayName::Marklin6050::centralUnitVersion);
  Attributes::addHelp(centralUnitVersion, "marklin6050_settings:central_unit_version.help");
  Attributes::addEnabled(centralUnitVersion, true);
  Attributes::addValues(centralUnitVersion, std::span<const uint16_t>{kCuVersionValues});
  Attributes::addAliases(centralUnitVersion,
    std::span<const uint16_t>{kCuVersionValues},
    std::span<const std::string_view>{kCuVersionLabels});
  m_interfaceItems.add(centralUnitVersion);

  // analog — only enabled for units that support it (6027/6029); toggled by centralUnitVersionChanged
  Attributes::addCategory(analog, "category:marklin_6050");
  Attributes::addDisplayName(analog, DisplayName::Marklin6050::analog);
  Attributes::addHelp(analog, "marklin6050_settings:analog.help");
  Attributes::addEnabled(analog, false);
  m_interfaceItems.add(analog);

  // s88amount — disabled when online
  Attributes::addCategory(s88amount, "category:marklin_6050");
  Attributes::addDisplayName(s88amount, DisplayName::Marklin6050::s88ModuleAmount);
  Attributes::addHelp(s88amount, "marklin6050_settings:s88_module_amount.help");
  Attributes::addEnabled(s88amount, true);
  Attributes::addMinMax(s88amount, 0u, 61u);
  m_interfaceItems.add(s88amount);

  // s88interval — disabled when online
  Attributes::addCategory(s88interval, "category:marklin_6050");
  Attributes::addDisplayName(s88interval, DisplayName::Marklin6050::s88CallInterval);
  Attributes::addHelp(s88interval, "marklin6050_settings:s88_call_interval.help");
  Attributes::addEnabled(s88interval, true);
  Attributes::addValues(s88interval, std::span<const unsigned int>{kS88Intervals});
  Attributes::addAliases(s88interval,
    std::span<const unsigned int>{kS88Intervals},
    std::span<const std::string_view>{kS88IntervalLabels});
  m_interfaceItems.add(s88interval);

  // turnouttime — disabled for 6021 (which handles timing internally) and when online
  Attributes::addCategory(turnouttime, "category:marklin_6050");
  Attributes::addDisplayName(turnouttime, DisplayName::Marklin6050::accessoryOffTime);
  Attributes::addHelp(turnouttime, "marklin6050_settings:accessory_off_time.help");
  Attributes::addEnabled(turnouttime, true);
  Attributes::addValues(turnouttime, std::span<const unsigned int>{kTurnoutTimes});
  Attributes::addAliases(turnouttime,
    std::span<const unsigned int>{kTurnoutTimes},
    std::span<const std::string_view>{kTurnoutTimeLabels});
  m_interfaceItems.add(turnouttime);

  // redundancy — disabled when online
  Attributes::addCategory(redundancy, "category:marklin_6050");
  Attributes::addDisplayName(redundancy, DisplayName::Marklin6050::commandRedundancy);
  Attributes::addHelp(redundancy, "marklin6050_settings:command_redundancy.help");
  Attributes::addEnabled(redundancy, true);
  Attributes::addValues(redundancy, std::span<const unsigned int>{kRedundancyOptions});
  Attributes::addAliases(redundancy,
    std::span<const unsigned int>{kRedundancyOptions},
    std::span<const std::string_view>{kRedundancyLabels});
  m_interfaceItems.add(redundancy);

  // extensions — disabled when online
  Attributes::addCategory(extensions, "category:marklin_6050");
  Attributes::addDisplayName(extensions, DisplayName::Marklin6050::feedbackModule);
  Attributes::addHelp(extensions, "marklin6050_settings:feedback_module.help");
  Attributes::addEnabled(extensions, true);
  m_interfaceItems.add(extensions);

  // commandQueue — enable the paced command queue; disabled when online
  Attributes::addCategory(commandQueue, "category:marklin_6050");
  Attributes::addDisplayName(commandQueue, DisplayName::Marklin6050::commandQueue);
  Attributes::addHelp(commandQueue, "marklin6050_settings:command_queue.help");
  Attributes::addEnabled(commandQueue, true);
  m_interfaceItems.add(commandQueue);

  // commandInterval — ms between queued sends; only enabled while commandQueue is on
  Attributes::addCategory(commandInterval, "category:marklin_6050");
  Attributes::addDisplayName(commandInterval, DisplayName::Marklin6050::commandInterval);
  Attributes::addHelp(commandInterval, "marklin6050_settings:command_interval.help");
  Attributes::addEnabled(commandInterval, commandQueue);
  Attributes::addMinMax(commandInterval, 10u, 500u);
  Attributes::addStep(commandInterval, 10u);
  Attributes::addUnit(commandInterval, Unit::milliSeconds);
  m_interfaceItems.add(commandInterval);

  // ignoreWarnings — disable the auto power-off on crash/overflow (still logged)
  Attributes::addCategory(ignoreWarnings, "category:marklin_6050");
  Attributes::addDisplayName(ignoreWarnings, DisplayName::Marklin6050::ignoreWarnings);
  Attributes::addHelp(ignoreWarnings, "marklin6050_settings:ignore_warnings.help");
  Attributes::addEnabled(ignoreWarnings, true);
  m_interfaceItems.add(ignoreWarnings);

  // crashDetection — monitor CTS and stop the world on probable crash/disconnect
  Attributes::addCategory(crashDetection, "category:marklin_6050");
  Attributes::addDisplayName(crashDetection, DisplayName::Marklin6050::crashDetection);
  Attributes::addHelp(crashDetection, "marklin6050_settings:crash_detection.help");
  Attributes::addEnabled(crashDetection, true);
  m_interfaceItems.add(crashDetection);

  // waitForCts — queue waits for CTS; only enabled while crash detection is on
  Attributes::addCategory(waitForCts, "category:marklin_6050");
  Attributes::addDisplayName(waitForCts, DisplayName::Marklin6050::waitForCts);
  Attributes::addHelp(waitForCts, "marklin6050_settings:wait_for_cts.help");
  Attributes::addEnabled(waitForCts, crashDetection);
  m_interfaceItems.add(waitForCts);

  // debugLogRXTX — "log all communication"; always enabled, shown last
  Attributes::addCategory(debugLogRXTX, "category:marklin_6050");
  Attributes::addDisplayName(debugLogRXTX, DisplayName::Hardware::debugLogRXTX);
  m_interfaceItems.add(debugLogRXTX);
}

Config Settings::config() const
{
  Config cfg;
  cfg.centralUnitVersion = centralUnitVersion;
  cfg.analog             = analog;
  cfg.s88amount          = s88amount;
  cfg.s88interval        = s88interval;
  cfg.turnouttime        = turnouttime;
  cfg.redundancy         = redundancy;
  cfg.extensions         = extensions;
  cfg.debugLogRXTX       = debugLogRXTX;
  cfg.commandQueue       = commandQueue;
  cfg.commandInterval    = commandInterval;
  cfg.ignoreWarnings     = ignoreWarnings;
  cfg.crashDetection     = crashDetection;
  cfg.waitForCts         = waitForCts;
  return cfg;
}

void Settings::loaded()
{
  SubObject::loaded();
  centralUnitVersionChanged(centralUnitVersion);
}

void Settings::updateEnabled(bool online)
{
  Attributes::setEnabled(centralUnitVersion, !online);
  Attributes::setEnabled(s88amount,          !online);
  Attributes::setEnabled(s88interval,        !online);
  Attributes::setEnabled(redundancy,         !online);
  Attributes::setEnabled(extensions,         !online);
  Attributes::setEnabled(commandQueue,       !online);
  Attributes::setEnabled(commandInterval,    commandQueue && !online);
  Attributes::setEnabled(ignoreWarnings,     !online);
  Attributes::setEnabled(crashDetection,     !online);
  Attributes::setEnabled(waitForCts,         crashDetection && !online);

  const uint16_t ver          = centralUnitVersion;
  const bool     analogSupport = (ver == 6027 || ver == 6029);

  Attributes::setEnabled(analog,      analogSupport && !online);
  Attributes::setEnabled(turnouttime, (ver != 6021) && !online);

  if(!analogSupport)
  {
    analog.setValueInternal(false);
  }
}

void Settings::centralUnitVersionChanged(uint16_t /*value*/)
{
  updateEnabled(false);
}

} // namespace Marklin6050
