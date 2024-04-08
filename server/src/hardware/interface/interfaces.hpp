/**
 * server/src/hardware/interface/interfaces.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_INTERFACES_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_INTERFACES_HPP

#include "interface.hpp"
#include "../../utils/makearray.hpp"

#include "dccexinterface.hpp"
#include "ecosinterface.hpp"
#include "hsi88.hpp"
#include "loconetinterface.hpp"
#include "marklincaninterface.hpp"
#include "traintasticdiyinterface.hpp"
#include "withrottleinterface.hpp"
#include "wlanmausinterface.hpp"
#include "xpressnetinterface.hpp"
#include "z21interface.hpp"

struct Interfaces
{
  static constexpr std::string_view classIdPrefix = "interface.";

  static constexpr auto classList = makeArray(
    DCCEXInterface::classId,
    ECoSInterface::classId,
    HSI88Interface::classId,
    LocoNetInterface::classId,
    MarklinCANInterface::classId,
    TraintasticDIYInterface::classId,
    WiThrottleInterface::classId,
    WlanMausInterface::classId,
    XpressNetInterface::classId,
    Z21Interface::classId
  );

  static std::shared_ptr<Interface> create(World& world, std::string_view classId, std::string_view id = std::string_view{});
};

#endif
