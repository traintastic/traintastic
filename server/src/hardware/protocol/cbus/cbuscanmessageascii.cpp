/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#include "cbuscanmessageascii.hpp"
#include <array>
#include "../../../utils/fromchars.hpp"

namespace CBUS {

std::size_t toAscii(const CAN::Message& canMessage, std::span<char> buffer)
{
  static constexpr std::array<char, 16> hexChars{{
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
  }};

  const size_t size = 2 + (canMessage.extended ? 8 : 4) + 1 + (canMessage.rtr ? 1 : canMessage.dlc * 2) + 1;
  if(buffer.size() < size)
  {
    return 0;
  }

  char* p = buffer.data();

  *(p++) = ':';

  if(canMessage.extended)
  {
    // UNTESTED: just a guess, not meantioned in the "CBUS Developer’s Guide Rev 6c"
    const auto eid = canMessage.id << 3;
    *(p++) = 'X';
    *(p++) = hexChars[(eid >> 28) & 0xF];
    *(p++) = hexChars[(eid >> 24) & 0xF];
    *(p++) = hexChars[(eid >> 20) & 0xF];
    *(p++) = hexChars[(eid >> 16) & 0xF];
    *(p++) = hexChars[(eid >> 12) & 0xF];
    *(p++) = hexChars[(eid >> 8) & 0xF];
    *(p++) = hexChars[(eid >> 4) & 0xF];
    *(p++) = hexChars[eid & 0xF];
  }
  else // standard
  {
    const auto sid = static_cast<uint16_t>(canMessage.id << 5); // align high, to map directly to SIDH and SIDL.
    *(p++) = 'S';
    *(p++) = hexChars[(sid >> 12) & 0xF];
    *(p++) = hexChars[(sid >> 8) & 0xF];
    *(p++) = hexChars[(sid >> 4) & 0xF];
    *(p++) = '0';
  }

  if(canMessage.rtr)
  {
    *(p++) = 'R';
    *(p++) = '0' + canMessage.dlc;
  }
  else // normal
  {
    *(p++) = 'N';
    for(uint8_t i = 0; i < canMessage.dlc; ++i)
    {
      *(p++) = hexChars[canMessage.data[i] >> 4];
      *(p++) = hexChars[canMessage.data[i] & 0x0F];
    }
  }

  *(p++) = ';';

  return size;
}

std::size_t fromAscii(std::string_view buffer, CAN::Message& canMessage, std::size_t& dropped)
{
  std::size_t consumed = 0;

  while(consumed < buffer.size())
  {
    if(auto pos = buffer.find(':'); pos != 0)
    {
      if(pos == std::string_view::npos)
      {
        // no start marker drop all remaining bytes:
        dropped += buffer.size();
        consumed += buffer.size();
        return consumed;
      }

      // drop bytes before start marker:
      dropped += pos;
      consumed += pos;
      buffer.remove_prefix(pos);
    }

    auto end = buffer.find(';');
    if(end == std::string_view::npos)
    {
      // no end marker yet, wait for more data
      return consumed;
    }

    std::string_view frame{buffer.data(), end + 1};
    consumed += frame.size();

    if(frame[1] == 'S' && (frame[6] == 'N' || frame[6] == 'R')) // standard
    {
      uint16_t sid;
      if(fromChars(frame.substr(2, sizeof(sid) * 2), sid, 16).ec != std::errc())
      {
        dropped += frame.size();
        continue; // error reading, ignore frame
      }
      canMessage.id = (sid >> 5);
      canMessage.extended = false;
      canMessage.rtr = (frame[6] == 'R');
      if(!canMessage.rtr)
      {
        canMessage.dlc = (frame.size() - 7) / 2;
      }
    }
    else if(frame[1] == 'X' && (frame[10] == 'N' || frame[10] == 'R')) // extended
    {
      uint32_t eid;
      if(fromChars(frame.substr(2, sizeof(eid) * 2), eid, 16).ec != std::errc())
      {
        dropped += frame.size();
        continue; // error reading, ignore frame
      }
      canMessage.id = (eid >> 3);
      canMessage.extended = true;
      canMessage.rtr = (frame[10] == 'R');
      if(!canMessage.rtr)
      {
        canMessage.dlc = (frame.size() - 11) / 2;
      }
    }
    else
    {
      dropped += frame.size();
      continue; // error reading, ignore frame
    }

    if(canMessage.rtr)
    {
      // FIXME
    }
    else // normal
    {
      std::errc ec = std::errc();
      for(size_t i = 0; i < canMessage.dlc; ++i)
      {
        ec = fromChars(frame.substr((canMessage.extended ? 11 : 7) + i * 2, 2), canMessage.data[i], 16).ec;
        if(ec != std::errc())
        {
          break;
        }
      }
      if(ec != std::errc())
      {
        dropped += frame.size();
        continue; // error reading, ignore frame
      }
    }

    // we got a frame
    return consumed;
  }
  return consumed;
}

}


