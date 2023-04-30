/**
 * shared/src/traintastic/enum/pcapoutput.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_PCAPOUTPUT_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_PCAPOUTPUT_HPP

#include <cstdint>
#include <array>
#include "enum.hpp"

enum class PCAPOutput : uint8_t
{
  File = 1,
  Pipe = 2,
};

TRAINTASTIC_ENUM(PCAPOutput, "pcap_output", 2,
{
  {PCAPOutput::File, "file"},
  {PCAPOutput::Pipe, "pipe"},
});

inline constexpr std::array<PCAPOutput, 2> pcapOutputValues{{
  PCAPOutput::File,
  PCAPOutput::Pipe,
}};

#endif