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
            else if(typ == "mm2_dil8" || typ == "mm2_prg")
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
          else if(line == "funktionen")
          {
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
                  uint8_t typ;
                  if(fromChars(line.substr(4), typ).ec == std::errc())
                  {
                    (void)typ; //! \todo convert value to type/function (if constant)
                  }
                }
              }
              else
              {
                list = {line.data(), line.size() + 1 + list.size()}; // restore list for next readLine
                break;
              }
            }

            if(function.nr != 0xFF)
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
