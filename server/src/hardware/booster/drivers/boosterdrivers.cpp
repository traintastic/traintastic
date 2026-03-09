/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2025-2026 Reinder Feenstra
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

#include "boosterdrivers.hpp"
#include "../../../utils/stripprefix.hpp"

#include "dr5033boosterdriver.hpp"
#include "power4boosterdriver.hpp"
#include "power7boosterdriver.hpp"
#include "power22boosterdriver.hpp"
#include "power40boosterdriver.hpp"
#include "power70boosterdriver.hpp"

#define BOOSTER_DRIVERS \
  BOOSTER_DRIVER(DR5033BoosterDriver) \
  BOOSTER_DRIVER(Power4BoosterDriver) \
  BOOSTER_DRIVER(Power7BoosterDriver) \
  BOOSTER_DRIVER(Power22BoosterDriver) \
  BOOSTER_DRIVER(Power40BoosterDriver) \
  BOOSTER_DRIVER(Power70BoosterDriver)

std::span<const std::string_view> BoosterDrivers::types()
{
  static constexpr auto values = std::array{
#define BOOSTER_DRIVER(T) stripPrefix(T::classId, classIdPrefix),
    BOOSTER_DRIVERS
#undef BOOSTER_DRIVER
  };
  return values;
}

std::span<const std::string_view> BoosterDrivers::names()
{
  static constexpr auto values = std::array{
#define BOOSTER_DRIVER(T) T::name,
  BOOSTER_DRIVERS
#undef BOOSTER_DRIVER
  };
  return values;
}

std::shared_ptr<BoosterDriver> BoosterDrivers::create(std::string_view typeId, Booster& booster)
{
#define BOOSTER_DRIVER(T) if(typeId == stripPrefix(T::classId, classIdPrefix)) { return std::make_shared<T>(booster); }
  BOOSTER_DRIVERS
#undef BOOSTER_DRIVER
  return {};
}
