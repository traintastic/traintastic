/**
 * server/src/hardware/input/loconetinput.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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
#include "../../core/attributes.hpp"
#include "../../world/world.hpp"
#include "../../log/log.hpp"

LocoNetInput::LocoNetInput(const std::weak_ptr<World> world, std::string_view _id) :
  Input(world, _id),
  loconet{this, "loconet", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](const std::shared_ptr<LocoNet::LocoNet>& value)
    {
      if(!value || value->addInput(*this))
      {
        if(loconet.value())
          loconet->removeInput(*this);
        return true;
      }
      return false;
    }},
  address{this, "address", addressMin, PropertyFlags::ReadWrite | PropertyFlags::Store, nullptr,
    [this](const uint16_t& value)
    {
      if(loconet)
        return loconet->changeInputAddress(*this, value);
      return true;
    }}
{
  auto w = world.lock();
  const bool editable = w && contains(w->state.value(), WorldState::Edit);

  Attributes::addEnabled(loconet, editable);
  Attributes::addObjectList(loconet, w->loconets);
  m_interfaceItems.add(loconet);
  Attributes::addEnabled(address, editable);
  Attributes::addMinMax(address, addressMin, addressMax);
  m_interfaceItems.add(address);
}

void LocoNetInput::loaded()
{
  Input::loaded();
  if(loconet)
  {
    if(!loconet->addInput(*this))
    {
      Log::log(*this, LogMessage::C2001_ADDRESS_ALREADY_USED_AT_X, *loconet);
      loconet.setValueInternal(nullptr);
    }
  }
}

void LocoNetInput::destroying()
{
  if(loconet)
    loconet = nullptr;
  Input::destroying();
}

void LocoNetInput::worldEvent(WorldState state, WorldEvent event)
{
  Input::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(loconet, editable);
  Attributes::setEnabled(address, editable);
}

void LocoNetInput::idChanged(const std::string& id)
{
  if(loconet)
    loconet->inputMonitorIdChanged(address, id);
}

void LocoNetInput::valueChanged(TriState _value)
{
  if(loconet)
    loconet->inputMonitorValueChanged(address, _value);
}
