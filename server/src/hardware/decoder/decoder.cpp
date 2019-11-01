/**
 * Traintastic
 *
 * Copyright (C) 2019 Reinder Feenstra <reinderfeenstra@gmail.com>
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

#include "decoder.hpp"
#include "decoderfunction.hpp"

namespace Hardware {

Decoder::Decoder(const std::string& _id) :
  IdObject(_id),
  protocol{this, "protocol", DecoderProtocol::None, PropertyFlags::AccessWCC},
  address{this, "address", 0, PropertyFlags::AccessWCC},
  emergencyStop{this, "emergency_stop", false, PropertyFlags::AccessRWW},
  direction{this, "direction", Direction::Forward, PropertyFlags::AccessWWW},
  speedSteps{this, "speed_steps", 255, PropertyFlags::AccessWCC},
  speedStep{this, "speed_step", 0, PropertyFlags::AccessRRW}
{
}

const std::shared_ptr<DecoderFunction>& Decoder::getFunction(uint32_t number) const
{
  return DecoderFunction::null;
}

}
