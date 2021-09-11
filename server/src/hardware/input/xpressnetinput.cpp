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
#include "../../log/log.hpp"
#include "../../utils/displayname.hpp"

XpressNetInput::XpressNetInput(const std::weak_ptr<World> world, std::string_view _id) :
  Input(world, _id),
  xpressnet{this, "xpressnet", nullptr, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](const std::shared_ptr<XpressNet::XpressNet>& newValue)
    {
      if(!newValue || newValue->addInput(*this))
      {
        if(xpressnet.value())
          xpressnet->removeInput(*this);
        return true;
      }
      return false;
    }},
  address{this, "address", addressMin, PropertyFlags::ReadWrite | PropertyFlags::Store, nullptr,
    [this](const uint16_t& newValue)
    {
      if(xpressnet)
        return xpressnet->changeInputAddress(*this, newValue);
      return true;
    }}
{
  auto w = world.lock();
  const bool editable = w && contains(w->state.value(), WorldState::Edit);

  Attributes::addDisplayName(xpressnet, DisplayName::Hardware::xpressnet);
  Attributes::addEnabled(xpressnet, editable);
  Attributes::addObjectList(xpressnet, w->xpressnets);
  m_interfaceItems.add(xpressnet);

  Attributes::addDisplayName(address, DisplayName::Hardware::address);
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
      Log::log(*this, LogMessage::C2001_ADDRESS_ALREADY_USED_AT_X, *xpressnet);
      xpressnet.setValueInternal(nullptr);
    }
  }
}

void XpressNetInput::destroying()
{
  if(xpressnet)
    xpressnet = nullptr;
  Input::destroying();
}

void XpressNetInput::worldEvent(WorldState state, WorldEvent event)
{
  Input::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(xpressnet, editable);
  Attributes::setEnabled(address, editable);
}

void XpressNetInput::idChanged(const std::string& newId)
{
  if(xpressnet)
    xpressnet->inputMonitorIdChanged(address, newId);
}

void XpressNetInput::valueChanged(TriState _value)
{
  if(xpressnet)
    xpressnet->inputMonitorValueChanged(address, _value);
}
