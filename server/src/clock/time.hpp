/**
 * server/src/clock/time.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_CLOCK_TIME_HPP
#define TRAINTASTIC_SERVER_CLOCK_TIME_HPP

#include <cstdint>
#include <cassert>
#include <string>

struct Time
{
  static constexpr uint16_t ticksPerDay = 24 * 60;

  uint16_t ticks = 0; //!< Ticks since midnight, each tick is one fast clock minute.

  uint8_t hour() const
  {
    return ticks / 60;
  }

  uint8_t minute() const
  {
    return ticks % 60;
  }

  void set(uint8_t hour, uint8_t minute)
  {
    assert(hour < 24);
    assert(minute < 60);
    ticks = static_cast<uint16_t>(hour) * 60 + minute;
  }

  Time& operator ++()
  {
    if(++ticks == ticksPerDay)
      ticks = 0;
    return *this;
  }
};

inline std::string toString(Time time)
{
  return std::to_string(time.hour()).append(time.minute() < 10 ? ":0" : ":").append(std::to_string(time.minute()));
}

#endif
