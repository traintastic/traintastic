/**
 * server/src/hardware/protocol/z21.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

#include "z21.hpp"
#include "../decoder/decoder.hpp"

//namespace Z21 {

z21_lan_x_loco_info::z21_lan_x_loco_info(const Decoder& decoder) :
  z21_lan_x_loco_info()
{
  setAddress(decoder.address, decoder.longAddress);
  setSpeedSteps(decoder.speedSteps);
  setDirection(decoder.direction);
  if(decoder.emergencyStop)
    setEmergencyStop();
  else
    setSpeedStep(decoder.speedStep);
  for(auto function : *decoder.functions)
    setFunction(function->number, function->value);
  calcChecksum();
}

//}
