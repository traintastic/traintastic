/**
 * server/src/hardware/interface/interfaces.cpp
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

#include "interfaces.hpp"
#include "../../utils/ifclassidcreate.hpp"
#include "../../world/world.hpp"

std::shared_ptr<Interface> Interfaces::create(World& world, std::string_view classId, std::string_view id)
{
  IF_CLASSID_CREATE(DCCPlusPlusInterface)
  IF_CLASSID_CREATE(ECoSInterface)
  IF_CLASSID_CREATE(HSI88Interface)
  IF_CLASSID_CREATE(LocoNetInterface)
  IF_CLASSID_CREATE(MarklinCANInterface)
  IF_CLASSID_CREATE(TraintasticDIYInterface)
  IF_CLASSID_CREATE(WiThrottleInterface)
  IF_CLASSID_CREATE(WlanMausInterface)
  IF_CLASSID_CREATE(XpressNetInterface)
  IF_CLASSID_CREATE(Z21Interface)
  return std::shared_ptr<Interface>();
}
