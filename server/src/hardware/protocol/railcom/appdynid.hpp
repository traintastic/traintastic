/**
 * server/src/hardware/protocol/railcom/appdynid.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_RAILCOM_APPDYNID_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_RAILCOM_APPDYNID_HPP

#include <cstdint>
#include "../../../utils/inrange.hpp"

namespace RailCom {

enum class AppDynId : uint8_t
{
  ActualSpeed = 0,
  ActualSpeedHigh = 1,
  QualityOfService = 7,
  Container1 = 8,
  Container2 = 9,
  Container3 = 10,
  Container4 = 11,
  Container5 = 12,
  Container6 = 13,
  Container7 = 14,
  Container8 = 15,
  Container9 = 16,
  Container10 = 17,
  Container11 = 18,
  Container12 = 19,
};

constexpr bool isAppDynActualSpeed(AppDynId value)
{
  return
    (value == RailCom::AppDynId::ActualSpeed) ||
    (value == RailCom::AppDynId::ActualSpeedHigh);
}

constexpr bool isAppDynContainer(AppDynId value)
{
  return inRange(
    static_cast<uint8_t>(value),
    static_cast<uint8_t>(RailCom::AppDynId::Container1),
    static_cast<uint8_t>(RailCom::AppDynId::Container12));
}

}

constexpr std::string_view toString(RailCom::AppDynId value)
{
  switch(value)
  {
    case RailCom::AppDynId::ActualSpeed:
      return "ActualSpeed";

    case RailCom::AppDynId::ActualSpeedHigh:
      return "ActualSpeedHigh";

    case RailCom::AppDynId::QualityOfService:
      return "QualityOfService";

    case RailCom::AppDynId::Container1:
      return "Container1";

    case RailCom::AppDynId::Container2:
      return "Container2";

    case RailCom::AppDynId::Container3:
      return "Container3";

    case RailCom::AppDynId::Container4:
      return "Container4";

    case RailCom::AppDynId::Container5:
      return "Container5";

    case RailCom::AppDynId::Container6:
      return "Container6";

    case RailCom::AppDynId::Container7:
      return "Container7";

    case RailCom::AppDynId::Container8:
      return "Container8";

    case RailCom::AppDynId::Container9:
      return "Container9";

    case RailCom::AppDynId::Container10:
      return "Container10";

    case RailCom::AppDynId::Container11:
      return "Container11";

    case RailCom::AppDynId::Container12:
      return "Container12";
  }
  return {};
}

#endif
