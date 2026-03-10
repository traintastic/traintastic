/**
 * server/src/hardware/protocol/Marklin6050Interface/settings.cpp
 *
 * Copyright (C) 2025
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "settings.hpp"
#include "../../../core/attributes.hpp"
#include "../../../utils/displayname.hpp"

namespace Marklin6050 {

Settings::Settings(Object& _parent, std::string_view parentPropertyName)
    : SubObject(_parent, parentPropertyName)
    , centralUnitVersion{this, "central_unit_version", 6020,
          PropertyFlags::ReadWrite | PropertyFlags::Store,
          [this](const uint16_t& value) { centralUnitVersionChanged(value); }}
    , analog{this, "analog", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
    , s88amount{this, "s88amount", 1, PropertyFlags::ReadWrite | PropertyFlags::Store}
    , s88interval{this, "s88interval", 400, PropertyFlags::ReadWrite | PropertyFlags::Store}
    , turnouttime{this, "turnouttime", 200, PropertyFlags::ReadWrite | PropertyFlags::Store}
    , redundancy{this, "redundancy", 0, PropertyFlags::ReadWrite | PropertyFlags::Store}
    , extensions{this, "extensions", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
    //, oldAddress{this, "oldAddress", 1, PropertyFlags::ReadWrite | PropertyFlags::Store}
    //, newAddress{this, "newAddress", 1, PropertyFlags::ReadWrite | PropertyFlags::Store}
    //, programmer{this, "programmer", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
    // --- Central Unit Version ---
    static const std::vector<uint16_t> cuOptions = {
        6020, 6021, 6022, 6023, 6223, 6027, 6029, 6030, 6032
    };
    static const std::vector<std::string_view> cuLabels = {
        "6020", "6021", "6022", "6023", "6223", "6027", "6029", "6030", "6032"
    };
    Attributes::addCategory(centralUnitVersion, "category:marklin_6050");
    Attributes::addDisplayName(centralUnitVersion, DisplayName::Marklin6050::centralUnitVersion);
    Attributes::addHelp(centralUnitVersion, "marklin6050_settings:central_unit_version.help");
    Attributes::addEnabled(centralUnitVersion, true);
    Attributes::addVisible(centralUnitVersion, true);
    Attributes::addValues(centralUnitVersion, &cuOptions);
    Attributes::addAliases(centralUnitVersion, &cuOptions, &cuLabels);
    m_interfaceItems.add(centralUnitVersion);

    // --- Analog mode ---
    Attributes::addCategory(analog, "category:marklin_6050");
    Attributes::addDisplayName(analog, DisplayName::Marklin6050::analog);
    Attributes::addHelp(analog, "marklin6050_settings:analog.help");
    Attributes::addEnabled(analog, false);
    Attributes::addVisible(analog, true);
    m_interfaceItems.add(analog);

    // --- S88 module amount ---
    Attributes::addCategory(s88amount, "category:marklin_6050");
    Attributes::addDisplayName(s88amount, DisplayName::Marklin6050::s88ModuleAmount);
    Attributes::addHelp(s88amount, "marklin6050_settings:s88_module_amount.help");
    Attributes::addEnabled(s88amount, true);
    Attributes::addVisible(s88amount, true);
    Attributes::addMinMax(s88amount, 0u, 61u);
    m_interfaceItems.add(s88amount);

    // --- S88 interval ---
    static const std::vector<unsigned int> intervals = {
        50, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1500, 2000, 2500, 3000
    };
    static const std::vector<std::string_view> intervalLabels = {
        "50ms", "100ms", "200ms", "300ms", "400ms", "500ms", "600ms",
        "700ms", "800ms", "900ms", "1s", "1.5s", "2s", "2.5s", "3s"
    };
    Attributes::addCategory(s88interval, "category:marklin_6050");
    Attributes::addDisplayName(s88interval, DisplayName::Marklin6050::s88CallInterval);
    Attributes::addHelp(s88interval, "marklin6050_settings:s88_call_interval.help");
    Attributes::addEnabled(s88interval, true);
    Attributes::addVisible(s88interval, true);
    Attributes::addValues(s88interval, intervals);
    Attributes::addAliases(s88interval, &intervals, &intervalLabels);
    m_interfaceItems.add(s88interval);

    // --- Accessory OFF time ---
    static const std::vector<unsigned int> turnoutTimes = {
        25, 50, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000
    };
    static const std::vector<std::string_view> turnoutTimeLabels = {
        "25ms", "50ms", "100ms", "200ms", "300ms", "400ms",
        "500ms", "600ms", "700ms", "800ms", "900ms", "1s"
    };
    Attributes::addCategory(turnouttime, "category:marklin_6050");
    Attributes::addDisplayName(turnouttime, DisplayName::Marklin6050::accessoryOffTime);
    Attributes::addHelp(turnouttime, "marklin6050_settings:accessory_off_time.help");
    Attributes::addEnabled(turnouttime, true);
    Attributes::addVisible(turnouttime, true);
    Attributes::addValues(turnouttime, turnoutTimes);
    Attributes::addAliases(turnouttime, &turnoutTimes, &turnoutTimeLabels);
    m_interfaceItems.add(turnouttime);

    // --- Command redundancy ---
    static const std::vector<unsigned int> redundancyOptions = {0, 1, 2, 3};
    static const std::vector<std::string_view> redundancyLabels = {
        "$marklin6050_settings:redundancy_off$", 
        "x2",
        "x3",
        "x4"
    };
    Attributes::addCategory(redundancy, "category:marklin_6050");
    Attributes::addDisplayName(redundancy, DisplayName::Marklin6050::commandRedundancy);
    Attributes::addHelp(redundancy, "marklin6050_settings:command_redundancy.help");
    Attributes::addEnabled(redundancy, true);
    Attributes::addVisible(redundancy, true);
    Attributes::addValues(redundancy, redundancyOptions);
    Attributes::addAliases(redundancy, &redundancyOptions, &redundancyLabels);
    m_interfaceItems.add(redundancy);

    // --- Extensions ---
    Attributes::addCategory(extensions, "category:marklin_6050");
    Attributes::addDisplayName(extensions, DisplayName::Marklin6050::feedbackModule);
    Attributes::addHelp(extensions, "marklin6050_settings:feedback_module.help");
    Attributes::addEnabled(extensions, true);
    Attributes::addVisible(extensions, true);
    m_interfaceItems.add(extensions);

    // --- Programmer (disabled more research needed) ---
    /*
    Attributes::addCategory(oldAddress, "category:programmer");
    Attributes::addDisplayName(oldAddress, DisplayName::Marklin6050::oldLocoAddress);
    Attributes::addEnabled(oldAddress, false);
    Attributes::addVisible(oldAddress, true);
    Attributes::addMinMax(oldAddress, 1u, 79u);
    m_interfaceItems.add(oldAddress);

    Attributes::addCategory(newAddress, "category:programmer");
    Attributes::addDisplayName(newAddress, DisplayName::Marklin6050::newLocoAddress);
    Attributes::addEnabled(newAddress, false);
    Attributes::addVisible(newAddress, true);
    Attributes::addMinMax(newAddress, 1u, 79u);
    m_interfaceItems.add(newAddress);

    Attributes::addCategory(programmer, "category:programmer");
    Attributes::addDisplayName(programmer, DisplayName::Marklin6050::changeAddress);
    Attributes::addHelp(programmer, "marklin6050_settings:change_address.help");
    Attributes::addEnabled(programmer, false);
    Attributes::addVisible(programmer, true);
    m_interfaceItems.add(programmer);
    */
}

Config Settings::config() const
{
    Config cfg;
    cfg.centralUnitVersion = centralUnitVersion;
    cfg.analog = analog;
    cfg.s88amount = s88amount;
    cfg.s88interval = s88interval;
    cfg.turnouttime = turnouttime;
    cfg.redundancy = redundancy;
    cfg.extensions = extensions;
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
    Attributes::setEnabled(s88amount, !online);
    Attributes::setEnabled(s88interval, !online);
    Attributes::setEnabled(redundancy, !online);
    Attributes::setEnabled(extensions, !online);

    // programmer only when online
    //Attributes::setEnabled(oldAddress, online);
    //Attributes::setEnabled(newAddress, online);
    //Attributes::setEnabled(programmer, online);

    const uint16_t ver = centralUnitVersion;

    // analog only on 6027/6029
    const bool analogSupport = (ver == 6027 || ver == 6029);
    Attributes::setEnabled(analog, analogSupport && !online);
    if(!analogSupport)
        analog.setValueInternal(false);

    // 6021 handles turnout timing internally
    Attributes::setEnabled(turnouttime, (ver != 6021) && !online);
}

void Settings::centralUnitVersionChanged(uint16_t /*value*/)
{
    updateEnabled(false);
}

} // namespace Marklin6050
