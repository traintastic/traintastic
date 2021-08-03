/**
 * server/src/hardware/output/loconetoutput.cpp
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

#include "loconetoutput.hpp"
#include "../../core/attributes.hpp"
#include "../../world/world.hpp"
#include "../../log/log.hpp"

LocoNetOutput::LocoNetOutput(const std::weak_ptr<World> world, std::string_view _id) :
  Output(world, _id),
  loconet{this, "loconet", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](const std::shared_ptr<LocoNet::LocoNet>& value)
    {
      if(!value || value->addOutput(*this))
      {
        if(loconet.value())
          loconet->removeOutput(*this);
        return true;
      }
      return false;
    }},
  address{this, "address", addressMin, PropertyFlags::ReadWrite | PropertyFlags::Store, nullptr,
    [this](const uint16_t& value)
    {
      if(loconet)
        return loconet->changeOutputAddress(*this, value);
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

void LocoNetOutput::loaded()
{
  Output::loaded();
  if(loconet)
  {
    if(!loconet->addOutput(*this))
    {
      Log::log(*this, LogMessage::C2001_ADDRESS_ALREADY_USED_AT_X, *loconet);
      loconet.setValueInternal(nullptr);
    }
  }
}

void LocoNetOutput::worldEvent(WorldState state, WorldEvent event)
{
  Output::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  loconet.setAttributeEnabled(editable);
  address.setAttributeEnabled(editable);
}

void LocoNetOutput::idChanged(const std::string& id)
{
  if(loconet)
    loconet->outputKeyboardIdChanged(address, id);
}

void LocoNetOutput::valueChanged(TriState _value)
{
  if(loconet)
    loconet->outputKeyboardValueChanged(address, _value);
}

bool LocoNetOutput::setValue(TriState& _value)
{
  if(!loconet || _value == TriState::Undefined)
    return false;
  else
    return loconet->send(LocoNet::SwitchRequest(address - 1, _value == TriState::True));
}
