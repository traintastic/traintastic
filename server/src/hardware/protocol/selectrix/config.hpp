/**
 * server/src/hardware/protocol/selectrix/config.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023,2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_SELECTRIX_CONFIG_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_SELECTRIX_CONFIG_HPP

#include <chrono>
#include "addresstype.hpp"

namespace Selectrix {

struct Config
{
  std::chrono::milliseconds trackPowerPollInterval;
  std::chrono::milliseconds locomotivePollInterval;
  std::chrono::milliseconds feedbackPollInterval;
  bool debugLogRXTX;

  std::chrono::milliseconds pollInterval(AddressType addressType)
  {
    using namespace std::chrono_literals;

    switch(addressType)
    {
      case AddressType::TrackPower:
        return trackPowerPollInterval;

      case AddressType::Locomotive:
        return locomotivePollInterval;

      case AddressType::Feedback:
        return feedbackPollInterval;
    }
    assert(false);
    return 1s;
  }
};

}

#endif
