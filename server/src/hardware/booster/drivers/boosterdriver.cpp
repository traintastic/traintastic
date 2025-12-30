/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2025 Reinder Feenstra
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

#include "boosterdriver.hpp"
#include "../booster.hpp"

using namespace std::string_view_literals;

BoosterDriver::BoosterDriver(Booster& booster)
  : SubObject(booster, "driver"sv)
{
}

const std::string& BoosterDriver::boosterName() const
{
  return booster().name.value();
}

void BoosterDriver::invalidateAll()
{
  booster().load_.setValueInternal(Booster::noValue);
  booster().temperature.setValueInternal(Booster::noValue);
  booster().current.setValueInternal(Booster::noValue);
  booster().voltage.setValueInternal(Booster::noValue);
  booster().inputVoltage.setValueInternal(Booster::noValue);
}

void BoosterDriver::reportLoad(float value)
{
  assert(contains(supportedStatusValues(), SupportedStatusValues::Load));
  assert(value >= 0 || std::isnan(value));
  booster().load_.setValueInternal(value);
}

void BoosterDriver::reportTemperature(float value)
{
  assert(contains(supportedStatusValues(), SupportedStatusValues::Temperature));
  assert(std::isfinite(value) || std::isnan(value));
  booster().temperature.setValueInternal(value);
}

void BoosterDriver::reportCurrent(float value)
{
  assert(contains(supportedStatusValues(), SupportedStatusValues::Current));
  assert(value >= 0 || std::isnan(value));
  booster().current.setValueInternal(value);
}

void BoosterDriver::reportVoltage(float value)
{
  assert(contains(supportedStatusValues(), SupportedStatusValues::Voltage));
  assert(value >= 0 || std::isnan(value));
  booster().voltage.setValueInternal(value);
}

void BoosterDriver::reportInputVoltage(float value)
{
  assert(contains(supportedStatusValues(), SupportedStatusValues::InputVoltage));
  assert(value >= 0 || std::isnan(value));
  booster().inputVoltage.setValueInternal(value);
}

Booster& BoosterDriver::booster() const
{
  return static_cast<Booster&>(parent());
}
