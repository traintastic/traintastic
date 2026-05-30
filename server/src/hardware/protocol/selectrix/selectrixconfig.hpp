/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2023-2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_SELECTRIX_SELECTRIXCONFIG_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_SELECTRIX_SELECTRIXCONFIG_HPP

#include <chrono>
#include "selectrixaddresstype.hpp"

namespace Selectrix {

struct Config
{
  std::chrono::milliseconds trackPowerPollInterval;
  std::chrono::milliseconds locomotivePollInterval;
  std::chrono::milliseconds feedbackPollInterval;
  std::chrono::milliseconds accessoryPollInterval;
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

      case AddressType::Accessory:
        return accessoryPollInterval;
    }
    assert(false);
    return 1s;
  }
};

}

#endif
