/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2022-2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_OBJECT_ECOSSWITCHPROTOCOL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_ECOS_OBJECT_ECOSSWITCHPROTOCOL_HPP

#include <string_view>

namespace ECoS
{

enum class SwitchProtocol
{
  Unknown = 0,
  DCC = 1,
  Motorola = 2,
};

constexpr std::string_view toString(SwitchProtocol value)
{
  switch(value)
  {
    case SwitchProtocol::DCC:
      return "DCC";

    case SwitchProtocol::Motorola:
      return "MM";

    case SwitchProtocol::Unknown:
      break;
  }
  return {};
}

constexpr bool fromString(std::string_view text, SwitchProtocol& protocol)
{
  if(text == "MM")
    protocol = SwitchProtocol::Motorola;
  else if(text == "DCC")
    protocol = SwitchProtocol::DCC;
  else
    return false;
  return true;
}

}

#endif
