/**
 * server/src/core/speedlimitproperty.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024-2025 Reinder Feenstra
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

#include "speedlimitproperty.hpp"
#include <cassert>
#include "attributes.hpp"

constexpr std::array<double, 1> aliasKeys = {{SpeedLimitProperty::noLimitValue}};
inline static const std::array<std::string, 1> aliasValues{{"$speed_limit_property:no_limit$"}};

static std::span<const double> getValues(SpeedUnit unit)
{
  switch(unit)
  {
    case SpeedUnit::KiloMeterPerHour:
    {
      static const std::array<double, 20> values = {
        10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160,
        200, 250, 300,
        SpeedLimitProperty::noLimitValue
      };
      return values;
    }
    case SpeedUnit::MilePerHour:
    {
      static const std::array<double, 18> values = {
        5, 10, 15, 20, 25, 30, 35, 40, 45, 50,
        60, 70, 80, 90, 100,
        150, 200,
        SpeedLimitProperty::noLimitValue
      };
      return values;
    }
    case SpeedUnit::MeterPerSecond:
    {
      static const std::array<double, 20> values = {
        2, 4, 6, 8, 10, 12, 14, 16, 18, 20,
        25, 30, 35, 40, 45, 50,
        60, 70, 80,
        SpeedLimitProperty::noLimitValue
      };
      return values;
    }
  }
  return {};
}

SpeedLimitProperty::SpeedLimitProperty(Object& object, std::string_view name, double value, SpeedUnit unit, PropertyFlags flags)
  : SpeedProperty(object, name, value, unit, flags,
      [this](double /*value*/, SpeedUnit unit_)
      {
        if(m_attributeUnit != unit_)
        {
          updateAttributes();
        }
      })
{
  addAttributes();
}

SpeedLimitProperty::SpeedLimitProperty(Object& object, std::string_view name, double value, SpeedUnit unit, PropertyFlags flags, OnChanged onChanged)
  : SpeedProperty(object, name, value, unit, flags,
      [this, onChanged](double value_, SpeedUnit unit_)
      {
        if(m_attributeUnit != unit_)
        {
          updateAttributes();
        }
        onChanged(value_, unit_);
      })
{
  assert(onChanged);
  addAttributes();
}

void SpeedLimitProperty::addAttributes()
{
  auto values = getValues(unit());
  ::Attributes::addMin(*this, values.front());
  ::Attributes::addValues(*this, values);
  ::Attributes::addAliases(*this, std::span<const double>(aliasKeys), std::span<const std::string>(aliasValues));
  m_attributeUnit = m_unit;
}

void SpeedLimitProperty::updateAttributes()
{
  auto values = getValues(unit());
  ::Attributes::setMin(*this, values.front());
  ::Attributes::setValues(*this, values);
  m_attributeUnit = m_unit;
}
