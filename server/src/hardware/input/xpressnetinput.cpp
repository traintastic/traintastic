/**
 * server/src/hardware/input/xpressnetinput.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#include "xpressnetinput.hpp"
#include "../../core/attributes.hpp"
#include "../../world/world.hpp"

XpressNetInput::XpressNetInput(const std::weak_ptr<World> world, std::string_view _id) :
  Input(world, _id),
  xpressnet{this, "xpressnet", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](const std::shared_ptr<XpressNet::XpressNet>& value)
    {
      if(!value || value->addInput(*this))
      {
        if(xpressnet.value())
          xpressnet->removeInput(*this);
        return true;
      }
      return false;
    }},
  address{this, "address", addressMin, PropertyFlags::ReadWrite | PropertyFlags::Store, nullptr,
    [this](const uint16_t& value)
    {
      if(xpressnet)
        return xpressnet->changeInputAddress(*this, value);
      return true;
    }}
{
  auto w = world.lock();
  const bool editable = w && contains(w->state.value(), WorldState::Edit);

  Attributes::addEnabled(xpressnet, editable);
  Attributes::addObjectList(xpressnet, w->xpressnets);
  m_interfaceItems.add(xpressnet);
  Attributes::addEnabled(address, editable);
  Attributes::addMinMax(address, addressMin, addressMax);
  m_interfaceItems.add(address);
}

void XpressNetInput::loaded()
{
  Input::loaded();
  if(xpressnet)
  {
    if(!xpressnet->addInput(*this))
    {
      logCritical("address in use (" + xpressnet->getObjectId() + ")");
      xpressnet.setValueInternal(nullptr);
    }
  }
}

void XpressNetInput::worldEvent(WorldState state, WorldEvent event)
{
  Input::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  xpressnet.setAttributeEnabled(editable);
  address.setAttributeEnabled(editable);
}

void XpressNetInput::idChanged(const std::string& id)
{
  if(xpressnet)
    xpressnet->inputMonitorIdChanged(address, id);
}

void XpressNetInput::valueChanged(TriState _value)
{
  if(xpressnet)
    xpressnet->inputMonitorValueChanged(address, _value);
}
