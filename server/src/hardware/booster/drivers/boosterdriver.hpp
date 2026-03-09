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

#ifndef TRAINTASTIC_SERVER_HARDWARE_BOOSTER_DRIVERS_BOOSTERDRIVER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_BOOSTER_DRIVERS_BOOSTERDRIVER_HPP

#include "../../../core/subobject.hpp"

#define BOOSTER_DRIVER_CREATE_DEF \
  public: \
    static std::shared_ptr<BoosterDriver> create(Booster& booster)

#define BOOSTER_DRIVER_CREATE_IMPL(T) \
  std::shared_ptr<BoosterDriver> T::create(Booster& booster) \
  { \
    return std::make_shared<T>(booster); \
  }

#define BOOSTER_DRIVER_CREATE(T) \
  public: \
    static std::shared_ptr<BoosterDriver> create(Booster& booster) \
    { \
      return std::make_shared<T>(booster); \
    }

#define BOOSTER_DRIVER_NAME(Name) \
  public: \
    static constexpr std::string_view name = Name; \
    std::string_view getName() const override { return name; }

class Booster;

class BoosterDriver : public SubObject
{
public:
  enum class SupportedStatusValues
  {
    Load = 1 << 0,
    Temperature = 1 << 1,
    Current = 1 << 2,
    Voltage = 1 << 3,
    InputVoltage = 1 << 4,
  };

  virtual std::string_view getName() const = 0;
  virtual SupportedStatusValues supportedStatusValues() const = 0;

  virtual void worldEvent(WorldState /*state*/, WorldEvent /*event*/)
  {
  }

protected:
  static constexpr auto noValue = std::numeric_limits<float>::quiet_NaN();

  BoosterDriver(Booster& booster);

  const std::string& boosterName() const;

  void invalidateAll();
  void reportLoad(float value = noValue);
  void reportTemperature(float value = noValue);
  void reportCurrent(float value = noValue);
  void reportVoltage(float value = noValue);
  void reportInputVoltage(float value = noValue);

private:
  Booster& booster() const;
};


constexpr BoosterDriver::SupportedStatusValues operator| (BoosterDriver::SupportedStatusValues lhs, BoosterDriver::SupportedStatusValues rhs)
{
  using UT = std::underlying_type_t<BoosterDriver::SupportedStatusValues>;
  return static_cast<BoosterDriver::SupportedStatusValues>(static_cast<UT>(lhs) | static_cast<UT>(rhs));
}

constexpr void operator|= (BoosterDriver::SupportedStatusValues& lhs, BoosterDriver::SupportedStatusValues rhs)
{
  lhs = lhs | rhs;
}

constexpr bool contains(BoosterDriver::SupportedStatusValues value, BoosterDriver::SupportedStatusValues mask)
{
  using UT = std::underlying_type_t<BoosterDriver::SupportedStatusValues>;
  return (static_cast<UT>(value) & static_cast<UT>(mask)) == static_cast<UT>(mask);
}

#endif
