/**
 * server/src/hardware/protocol/marklincan/locomotivelist.cpp
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

#include "locomotivelist.hpp"
#include <traintastic/enum/decoderprotocol.hpp>
#include "../dcc/dcc.hpp"
#include "../../../utils/fromchars.hpp"
#include "../../../utils/startswith.hpp"

namespace MarklinCAN {

static bool readLine(std::string_view& src, std::string_view& line)
{
  const size_t n = src.find('\n');
  if(n == std::string_view::npos)
    return false;

  line = src.substr(0, n);
  src = src.substr(n + 1);

  return true;
}

//! \brief Function types used by CS2 (and CS3?)
//! \note Values based on tests with CS2
enum class FunctionType : uint8_t
{
  Light = 1,
  CabLight = 48,
  F0 = 50,
  F1 = 51,
  F2 = 52,
  F3 = 53,
  F4 = 54,
  F5 = 55,
  F6 = 56,
  F7 = 57,
  F8 = 58,
  F9 = 59,
  F10 = 60,
  F11 = 61,
  F12 = 62,
  F13 = 63,
  F14 = 64,
  F15 = 65,
  F16 = 66,
  F17 = 67,
  F18 = 68,
  F19 = 69,
  F20 = 70,
  F21 = 71,
  F22 = 72,
  F23 = 73,
  F24 = 74,
  F25 = 75,
  F26 = 76,
  F27 = 77,
  F28 = 78,
  Mute = 109,
};

constexpr DecoderFunctionFunction toDecoderFunctionFunction(uint8_t typ)
{
  // lowest 7 bit only (at least for the CS2)
  switch(static_cast<FunctionType>(typ & 0x7F))
  {
    case FunctionType::Light:
      return DecoderFunctionFunction::Light;

    case FunctionType::Mute:
      return DecoderFunctionFunction::Mute;

    case FunctionType::CabLight:
    case FunctionType::F0:
    case FunctionType::F1:
    case FunctionType::F2:
    case FunctionType::F3:
    case FunctionType::F4:
    case FunctionType::F5:
    case FunctionType::F6:
    case FunctionType::F7:
    case FunctionType::F8:
    case FunctionType::F9:
    case FunctionType::F10:
    case FunctionType::F11:
    case FunctionType::F12:
    case FunctionType::F13:
    case FunctionType::F14:
    case FunctionType::F15:
    case FunctionType::F16:
    case FunctionType::F17:
    case FunctionType::F18:
    case FunctionType::F19:
    case FunctionType::F20:
    case FunctionType::F21:
    case FunctionType::F22:
    case FunctionType::F23:
    case FunctionType::F24:
    case FunctionType::F25:
    case FunctionType::F26:
    case FunctionType::F27:
    case FunctionType::F28:
      break; // Generic
  }
  return DecoderFunctionFunction::Generic;
}

constexpr DecoderFunctionType toDecoderFunctionType(uint8_t typ)
{
  // higest bit is used to indicate it is a momentary function (at least for the CS2)
  return (typ & 0x80) ? DecoderFunctionType::Momentary : DecoderFunctionType::OnOff;
}


LocomotiveList::LocomotiveList(std::string_view list)
  : m_locomotives{build(list)}
{
}

std::vector<LocomotiveList::Locomotive> LocomotiveList::build(std::string_view list)
{
  std::string_view line;

  if(list.empty() || !readLine(list, line) || line != "[lokomotive]")
    return {};

  std::vector<Locomotive> locomotives;
  while(readLine(list, line))
  {
    if(line == "lokomotive")
    {
      Locomotive locomotive;

      while(readLine(list, line))
      {
        if(startsWith(line, " ."))
        {
          line = line.substr(2);
          if(startsWith(line, "name="))
          {
            locomotive.name = line.substr(5);
          }
          else if(startsWith(line, "adresse=0x"))
          {
            fromChars(line.substr(10), locomotive.address, 16);
          }
          else if(startsWith(line, "typ="))
          {
            std::string_view typ = line.substr(4);
            if(typ == "mfx")
            {
              locomotive.protocol = DecoderProtocol::MFX;
            }
            else if(typ == "dcc")
            {
              locomotive.protocol = DecoderProtocol::DCCShort; // or DCCLong (handled later)
            }
            else if(typ == "mm2_dil8" || typ == "mm2_prg" || typ == "mm_prg")
            {
              locomotive.protocol = DecoderProtocol::Motorola;
            }
          }
          else if(startsWith(line, "sid=0x"))
          {
            fromChars(line.substr(6), locomotive.sid, 16);
          }
          else if(startsWith(line, "mfxuid=0x"))
          {
            fromChars(line.substr(9), locomotive.mfxUID, 16);
          }
          else if(line == "funktionen" || line == "funktionen_2")
          {
            bool hasTyp = false; // if typ is missing the function is unused
            Function function;
            function.nr = 0xFF;

            while(readLine(list, line))
            {
              if(startsWith(line, " .."))
              {
                line = line.substr(3);
                if(startsWith(line, "nr="))
                {
                  fromChars(line.substr(3), function.nr);
                }
                else if(startsWith(line, "typ="))
                {
                  hasTyp = true;
                  uint8_t typ;
                  if(fromChars(line.substr(4), typ).ec == std::errc())
                  {
                    function.type = toDecoderFunctionType(typ);
                    function.function = toDecoderFunctionFunction(typ);
                  }
                }
              }
              else
              {
                list = {line.data(), line.size() + 1 + list.size()}; // restore list for next readLine
                break;
              }
            }

            if(function.nr != 0xFF && hasTyp)
            {
              locomotive.functions.push_back(function);
            }
          }
        }
        else
        {
          list = {line.data(), line.size() + 1 + list.size()}; // restore list for next readLine
          break;
        }
      }

      if(locomotive.protocol == DecoderProtocol::DCCShort && DCC::isLongAddress(locomotive.address))
      {
        locomotive.protocol = DecoderProtocol::DCCLong;
      }

      locomotives.emplace_back(std::move(locomotive));
    }
  }

  return locomotives;
}

}
