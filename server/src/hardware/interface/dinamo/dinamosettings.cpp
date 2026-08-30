/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#include "dinamosettings.hpp"
#include "../../../core/attributes.hpp"
#include "../../../utils/displayname.hpp"
#include "../../../utils/unit.hpp"

namespace {

constexpr uint8_t hfiLevelMin = 0;
constexpr uint8_t hfiLevelMax = 15;

constexpr float pm32PulseDurationStep = 1000.f / 60.f;
constexpr float pm32PulseDurationMin = pm32PulseDurationStep;
constexpr float pm32PulseDurationDefault = 12 * pm32PulseDurationStep; //!< ~200ms
constexpr float pm32PulseDurationMax = 127 * pm32PulseDurationStep;

}

DinamoSettings::DinamoSettings(Object& _parent, std::string_view parentPropertyName)
  : SubObject(_parent, parentPropertyName)
  , setHFILevel{this, "set_hfi_level", false, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](bool value)
    {
      Attributes::setEnabled(hfiLevel, value);
    }}
  , hfiLevel{this, "hfi_level", hfiLevelMin, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , pm32PulseDuration{this, "pm32_pulse_duration", pm32PulseDurationDefault, PropertyFlags::ReadWrite | PropertyFlags::Store, nullptr,
      [](float& value)
      {
        value = std::round(value / pm32PulseDurationStep) * pm32PulseDurationStep;
        return true;
      }}
  , debugLogRXTX{this, "debug_log_rx_tx", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
  , debugLogTrainBlocks{this, "debug_log_train_blocks", false, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  m_interfaceItems.add(setHFILevel);

  Attributes::addEnabled(hfiLevel, setHFILevel);
  Attributes::addMinMax(hfiLevel, hfiLevelMin, hfiLevelMax);
  m_interfaceItems.add(hfiLevel);

  Attributes::addMinMax(pm32PulseDuration, pm32PulseDurationMin, pm32PulseDurationMax);
  Attributes::addStep(pm32PulseDuration, pm32PulseDurationStep);
  Attributes::addDecimals(pm32PulseDuration, 0);
  Attributes::addUnit(pm32PulseDuration, Unit::milliSeconds);
  m_interfaceItems.add(pm32PulseDuration);

  Attributes::addDisplayName(debugLogRXTX, DisplayName::Hardware::debugLogRXTX);
  //Attributes::addGroup(debugLogRXTX, Group::debug);
  m_interfaceItems.add(debugLogRXTX);

  //Attributes::addGroup(debugLogTrainBlocks, Group::debug);
  m_interfaceItems.add(debugLogTrainBlocks);
}

Dinamo::Config DinamoSettings::config() const
{
  return Dinamo::Config{
    .setHFILevel = setHFILevel,
    .hfiLevel = hfiLevelMin,
    .pm32PulseDuration = static_cast<uint8_t>(std::round(pm32PulseDuration / pm32PulseDurationStep)),
    .debugLogRXTX = debugLogRXTX,
  };
}

void DinamoSettings::loaded()
{
  SubObject::loaded();

  Attributes::setEnabled(hfiLevel, setHFILevel);
}
