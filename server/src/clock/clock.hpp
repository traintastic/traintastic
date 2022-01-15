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

#ifndef TRAINTASTIC_SERVER_CLOCK_CLOCK_HPP
#define TRAINTASTIC_SERVER_CLOCK_CLOCK_HPP

#include "../core/subobject.hpp"
#include "../core/property.hpp"

class Clock : public SubObject
{
  public:
    CLASS_ID("clock")

    Property<uint16_t> year;
    Property<uint8_t> month;
    Property<uint8_t> day;
    Property<uint8_t> hour;
    Property<uint8_t> minute;

    Property<uint16_t> multiplier;

    Clock(Object& _parent, std::string_view parentPropertyName);
};

#endif
