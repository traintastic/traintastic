/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2019-2026 Reinder Feenstra
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

#include "input.hpp"
#include "inputcontroller.hpp"
#include "list/inputlist.hpp"
#include "list/inputlisttablemodel.hpp"
#include "../../world/getworld.hpp"
#include "../../world/world.hpp"
#include "../../core/attributes.hpp"
#include "../../core/objectproperty.tpp"
#include "../../log/log.hpp"
#include "../../utils/displayname.hpp"

Input::Input(std::shared_ptr<InputController> inputController, InputChannel channel_, std::optional<uint32_t> node_, uint32_t address_)
  : interface{this, "interface", std::move(inputController), PropertyFlags::Constant | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly}
  , channel{this, "channel", channel_, PropertyFlags::Constant | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly}
  , address{this, "address", address_, PropertyFlags::Constant | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly}
  , node{this, "node", node_ ? *node_ : 0, PropertyFlags::Constant | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly}
  , value{this, "value", TriState::Undefined, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::ScriptReadOnly}
  , onValueChanged{*this, "on_value_changed", EventFlags::Scriptable}
{
  m_interfaceItems.add(interface);

  Attributes::addValues(channel, inputChannelValues);
  m_interfaceItems.add(channel);

  m_interfaceItems.add(address);

  if(node_)
  {
    m_interfaceItems.add(node);
  }

  Attributes::addValues(value, TriStateValues);
  m_interfaceItems.add(value);

  m_interfaceItems.add(onValueChanged);
}

void Input::simulateChange(SimulateInputAction action)
{
  assert(interface);
  assert(getWorld(dynamic_cast<Object*>(interface.value().get())).simulation.value()); // should only be called in simulation mode
  interface->inputSimulateChange(channel, inputLocation(channel, node, address), action);
}

void Input::updateValue(TriState _value)
{
  value.setValueInternal(_value);
  if(value != TriState::Undefined)
    fireEvent<bool, const std::shared_ptr<Input>&>(onValueChanged, value == TriState::True, shared_ptr<Input>());
}
