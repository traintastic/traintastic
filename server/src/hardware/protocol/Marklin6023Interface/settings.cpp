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

Settings::Settings(Object& parent, std::string_view parentPropertyName)
    : SubObject{parent, parentPropertyName}
    , s88amount{this, "s88amount", 1, PropertyFlags::ReadWrite | PropertyFlags::Store}
    , s88interval{this, "s88interval", 400, PropertyFlags::ReadWrite | PropertyFlags::Store}
    , redundancy{this, "redundancy", 0, PropertyFlags::ReadWrite | PropertyFlags::Store}
    , debugLogRXTX{this, "debug_log_rx_tx", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
    // --- S88 module count (max 4 for 6023/6223) ---
    Attributes::addCategory(s88amount, "category:marklin_6023");
    Attributes::addDisplayName(s88amount, DisplayName::Marklin6050::s88ModuleAmount);
    Attributes::addHelp(s88amount, "marklin6050_settings:s88_module_amount.help");
    Attributes::addEnabled(s88amount, true);
    Attributes::addVisible(s88amount, true);
    Attributes::addMinMax(s88amount, 0u, 4u);
    m_interfaceItems.add(s88amount);

    // --- S88 interval ---
    static const std::vector<unsigned int> intervals = {
        50, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1500, 2000, 2500, 3000
    };
    static const std::vector<std::string_view> intervalLabels = {
        "50ms","100ms","200ms","300ms","400ms","500ms","600ms",
        "700ms","800ms","900ms","1s","1.5s","2s","2.5s","3s"
    };
    Attributes::addCategory(s88interval, "category:marklin_6023");
    Attributes::addDisplayName(s88interval, DisplayName::Marklin6050::s88CallInterval);
    Attributes::addHelp(s88interval, "marklin6050_settings:s88_call_interval.help");
    Attributes::addEnabled(s88interval, true);
    Attributes::addVisible(s88interval, true);
    Attributes::addValues(s88interval, intervals);
    Attributes::addAliases(s88interval, &intervals, &intervalLabels);
    m_interfaceItems.add(s88interval);

    // --- Command redundancy ---
    static const std::vector<unsigned int> redundancyOptions = {0, 1, 2, 3};
    static const std::vector<std::string_view> redundancyLabels = {
        "$marklin6050_settings:redundancy_off$", "x2", "x3", "x4"
    };
    Attributes::addCategory(redundancy, "category:marklin_6023");
    Attributes::addDisplayName(redundancy, DisplayName::Marklin6050::commandRedundancy);
    Attributes::addHelp(redundancy, "marklin6050_settings:command_redundancy.help");
    Attributes::addEnabled(redundancy, true);
    Attributes::addVisible(redundancy, true);
    Attributes::addValues(redundancy, redundancyOptions);
    Attributes::addAliases(redundancy, &redundancyOptions, &redundancyLabels);
    m_interfaceItems.add(redundancy);

    // --- Debug log RX/TX ---
    Attributes::addCategory(debugLogRXTX, "category:marklin_6023");
    Attributes::addDisplayName(debugLogRXTX, DisplayName::Hardware::debugLogRXTX);
    Attributes::addEnabled(debugLogRXTX, true);
    m_interfaceItems.add(debugLogRXTX);
}

Config Settings::config() const
{
    Config cfg;
    cfg.s88amount   = s88amount;
    cfg.s88interval = s88interval;
    cfg.redundancy  = redundancy;
    cfg.debugLogRXTX = debugLogRXTX;
    return cfg;
}

void Settings::loaded()
{
    SubObject::loaded();
}

void Settings::updateEnabled(bool online)
{
    Attributes::setEnabled(s88amount,    !online);
    Attributes::setEnabled(s88interval,  !online);
    Attributes::setEnabled(redundancy,   !online);
    Attributes::setEnabled(debugLogRXTX, !online);
}

} // namespace Marklin6023
