/**
 * server/src/clock/clock.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

#include "clock.hpp"

Clock::Clock(Object& _parent, const std::string& parentPropertyName) :
  SubObject(_parent, parentPropertyName),
  year{this, "year", 2020, PropertyFlags::ReadWrite | PropertyFlags::StoreState},
  month{this, "month", 1, PropertyFlags::ReadWrite | PropertyFlags::StoreState},
  day{this, "day", 1, PropertyFlags::ReadWrite | PropertyFlags::StoreState},
  hour{this, "hour", 0, PropertyFlags::ReadWrite | PropertyFlags::StoreState},
  minute{this, "minute", 0, PropertyFlags::ReadWrite | PropertyFlags::StoreState},
  multiplier{this, "multiplier", 1, PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  m_interfaceItems.add(year);
  m_interfaceItems.add(month);
  m_interfaceItems.add(day);
  m_interfaceItems.add(hour);
  m_interfaceItems.add(minute);
  m_interfaceItems.add(multiplier);
}
