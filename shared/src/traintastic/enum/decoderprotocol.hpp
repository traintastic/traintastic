/**
 * shared/src/enum/decoderprotocol.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_DECODERPROTOCOL_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_DECODERPROTOCOL_HPP

#include <cstdint>
#include "enum.hpp"

enum class DecoderProtocol : uint8_t
{
  None = 0,
  DCCShort = 1,
  Motorola = 2,
  MFX = 3,
  Selectrix = 4,
  //FMZ = 5,
  DCCLong = 6,
};

TRAINTASTIC_ENUM(DecoderProtocol, "decoder_protocol", 6,
{
  {DecoderProtocol::None, "none"},
  {DecoderProtocol::DCCShort, "dcc_short"},
  {DecoderProtocol::Motorola, "motorola"},
  {DecoderProtocol::MFX, "mfx"},
  {DecoderProtocol::Selectrix, "selectrix"},
  {DecoderProtocol::DCCLong, "dcc_long"},
});

constexpr bool hasAddress(DecoderProtocol value)
{
  switch(value)
  {
    case DecoderProtocol::DCCShort:
    case DecoderProtocol::DCCLong:
    case DecoderProtocol::Motorola:
    case DecoderProtocol::Selectrix:
      return true;

    case DecoderProtocol::None:
    case DecoderProtocol::MFX:
      return false;
  }
  return false;
}

#endif
