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

namespace Marklin6023 {

// ---------------------------------------------------------------------------
// Attribute value arrays — defined here for stable span lifetimes
// ---------------------------------------------------------------------------

static constexpr std::array<unsigned int, 15> kS88Intervals{
  50, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1500, 2000, 2500, 3000
};
static constexpr std::array<std::string_view, 15> kS88IntervalLabels{
  "50ms","100ms","200ms","300ms","400ms","500ms","600ms",
  "700ms","800ms","900ms","1s","1.5s","2s","2.5s","3s"
};

static constexpr std::array<unsigned int, 4> kRedundancyOptions{0, 1, 2, 3};
static constexpr std::array<std::string_view, 4> kRedundancyLabels{
  "$marklin6023_settings:redundancy_off$", "x2", "x3", "x4"
};

// ---------------------------------------------------------------------------

Settings::Settings(Object& parent, std::string_view parentPropertyName)
  : SubObject{parent, parentPropertyName}
  , s88amount{this, "s88amount", 1, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , s88interval{this, "s88interval", 400, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , redundancy{this, "redundancy", 0, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , debugLogRXTX{this, "debug_log_rx_tx", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  // s88amount — disabled when online (toggled by updateEnabled)
  Attributes::addCategory(s88amount, "category:marklin_6023");
  Attributes::addDisplayName(s88amount, DisplayName::Marklin6050::s88ModuleAmount);
  Attributes::addHelp(s88amount, "marklin6050_settings:s88_module_amount.help");
  Attributes::addEnabled(s88amount, true);
  Attributes::addMinMax(s88amount, 0u, 4u);
  m_interfaceItems.add(s88amount);

  // s88interval — disabled when online
  Attributes::addCategory(s88interval, "category:marklin_6023");
  Attributes::addDisplayName(s88interval, DisplayName::Marklin6050::s88CallInterval);
  Attributes::addHelp(s88interval, "marklin6050_settings:s88_call_interval.help");
  Attributes::addEnabled(s88interval, true);
  Attributes::addValues(s88interval, std::span<const unsigned int>{kS88Intervals});
  Attributes::addAliases(s88interval,
    std::span<const unsigned int>{kS88Intervals},
    std::span<const std::string_view>{kS88IntervalLabels});
  m_interfaceItems.add(s88interval);

  // redundancy — disabled when online
  Attributes::addCategory(redundancy, "category:marklin_6023");
  Attributes::addDisplayName(redundancy, DisplayName::Marklin6050::commandRedundancy);
  Attributes::addHelp(redundancy, "marklin6050_settings:command_redundancy.help");
  Attributes::addEnabled(redundancy, true);
  Attributes::addValues(redundancy, std::span<const unsigned int>{kRedundancyOptions});
  Attributes::addAliases(redundancy,
    std::span<const unsigned int>{kRedundancyOptions},
    std::span<const std::string_view>{kRedundancyLabels});
  m_interfaceItems.add(redundancy);

  // debugLogRXTX — always enabled
  Attributes::addCategory(debugLogRXTX, "category:marklin_6023");
  Attributes::addDisplayName(debugLogRXTX, DisplayName::Hardware::debugLogRXTX);
  m_interfaceItems.add(debugLogRXTX);
}

Config Settings::config() const
{
  Config cfg;
  cfg.s88amount    = s88amount;
  cfg.s88interval  = s88interval;
  cfg.redundancy   = redundancy;
  cfg.debugLogRXTX = debugLogRXTX;
  return cfg;
}

void Settings::loaded()
{
  SubObject::loaded();
}

void Settings::updateEnabled(bool online)
{
  Attributes::setEnabled(s88amount,   !online);
  Attributes::setEnabled(s88interval, !online);
  Attributes::setEnabled(redundancy,  !online);
}

} // namespace Marklin6023
