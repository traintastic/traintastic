/**
 * shared/src/enum/loconetcommandstation.hpp
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_LOCONETCOMMANDSTATION_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_LOCONETCOMMANDSTATION_HPP

#include <cstdint>
#include "enum.hpp"
#include <frozen/map.h>

enum class LocoNetCommandStation : uint16_t
{
  Custom = 0,
  UhlenbrockIntellibox = 1,
  DigikeijsDR5000 = 2,
};

ENUM_NAME(LocoNetCommandStation, "loconet_command_station")

ENUM_VALUES(LocoNetCommandStation, 3,
{
  {LocoNetCommandStation::Custom, "custom"},
  {LocoNetCommandStation::UhlenbrockIntellibox, "uhlenbrock_intellibox"},
  {LocoNetCommandStation::DigikeijsDR5000, "digikeijs_dr5000"},
})

#endif
