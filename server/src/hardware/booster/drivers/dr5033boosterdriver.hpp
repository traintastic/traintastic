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

#ifndef TRAINTASTIC_SERVER_HARDWARE_BOOSTER_DRIVERS_DR5033BOOSTERDRIVER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_BOOSTER_DRIVERS_DR5033BOOSTERDRIVER_HPP

#include "loconetlncvboosterdriver.hpp"

class DR5033BoosterDriver final : public LocoNetLNCVBoosterDriver
{
  CLASS_ID("booster_driver.dr5033");
  BOOSTER_DRIVER_CREATE(DR5033BoosterDriver);
  BOOSTER_DRIVER_NAME("Digikeijs DR5033");

public:
  DR5033BoosterDriver(Booster& booster)
    : LocoNetLNCVBoosterDriver(booster, 5033)
  {
  }

  SupportedStatusValues supportedStatusValues() const final
  {
    using enum SupportedStatusValues;
    return (Load | Temperature);
  }
};

#endif

