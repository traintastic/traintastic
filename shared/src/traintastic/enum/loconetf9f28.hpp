/**
 * shared/src/traintastic/enum/loconetf9f28.hpp
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_LOCONETF9F28_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_LOCONETF9F28_HPP

#include <cstdint>
#include <array>
#include "enum.hpp"

enum class LocoNetF9F28 : uint16_t
{
  IMMPacket = 1,
  UhlenbrockExtended = 2,
};

TRAINTASTIC_ENUM(LocoNetF9F28, "loconet_f9_f28", 2,
{
  {LocoNetF9F28::IMMPacket, "imm_packet"},
  {LocoNetF9F28::UhlenbrockExtended, "uhlenbrock_extended"},
});

inline constexpr std::array<LocoNetF9F28, 2> loconetF9F28Values{{
  LocoNetF9F28::IMMPacket,
  LocoNetF9F28::UhlenbrockExtended,
}};

#endif