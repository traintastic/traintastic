/**
 * server/src/hardware/input/loconetinput.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2020 Reinder Feenstra
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

#include "loconetinput.hpp"

LocoNetInput::LocoNetInput(const std::weak_ptr<World> world, const std::string& _id) :
  Input(world, _id),
  loconet{this, "loconet", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](const std::shared_ptr<Protocol::LocoNet>& value)
    {
      if(!value || value->addInput(shared_ptr<LocoNetInput>()))
      {
        if(loconet.value())
          loconet->removeInput(shared_ptr<LocoNetInput>());
        return true;
      }
      return false;
    }},
  address{this, "address", 0, PropertyFlags::ReadWrite | PropertyFlags::Store, nullptr,
    [this](const uint16_t& value)
    {
      if(loconet)
        return loconet->isInputAddressAvailable(value);
      else
        return false;
    }}
{
  m_interfaceItems.add(loconet)
    .addAttributeEnabled(false);
  m_interfaceItems.add(address)
    .addAttributeEnabled(false);
}

void LocoNetInput::worldEvent(WorldState state, WorldEvent event)
{
  Input::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  loconet.setAttributeEnabled(editable);
  address.setAttributeEnabled(editable);
}
