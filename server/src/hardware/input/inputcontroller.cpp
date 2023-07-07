/**
 * server/src/hardware/input/inputcontroller.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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

#include "inputcontroller.hpp"
#include "input.hpp"
#include "list/inputlist.hpp"
#include "list/inputlisttablemodel.hpp"
#include "monitor/inputmonitor.hpp"
#include "../../core/attributes.hpp"
#include "../../core/controllerlist.hpp"
#include "../../core/objectproperty.tpp"
#include "../../utils/displayname.hpp"
#include "../../utils/inrange.hpp"
#include "../../world/world.hpp"

InputController::InputController(IdObject& interface)
  : inputs{&interface, "inputs", nullptr, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::SubObject}
{
  Attributes::addDisplayName(inputs, DisplayName::Hardware::inputs);
}

bool InputController::isInputChannel(uint32_t channel) const
{
  const auto* channels = inputChannels();
  if(!channels || channels->empty())
    return channel == defaultInputChannel;

  auto it = std::find(channels->begin(), channels->end(), channel);
  assert(it == channels->end() || *it != defaultInputChannel);
  return it != channels->end();
}

bool InputController::isInputAddressAvailable(uint32_t channel, uint32_t address) const
{
  assert(isInputChannel(channel));
  return
    inRange(address, inputAddressMinMax(channel)) &&
    m_inputs.find({channel, address}) == m_inputs.end();
}

uint32_t InputController::getUnusedInputAddress(uint32_t channel) const
{
  assert(isInputChannel(channel));
  const auto end = m_inputs.cend();
  const auto range = inputAddressMinMax(channel);
  for(uint32_t address = range.first; address < range.second; address++)
    if(m_inputs.find({channel, address}) == end)
      return address;
  return Input::invalidAddress;
}

bool InputController::changeInputChannelAddress(Input& input, uint32_t newChannel, uint32_t newAddress)
{
  assert(input.interface.value().get() == this);
  assert(isInputChannel(newChannel));

  if(!isInputAddressAvailable(newChannel, newAddress))
    return false;

  auto node = m_inputs.extract({input.channel, input.address});
  node.key() = {newChannel, newAddress};
  m_inputs.insert(std::move(node));
  input.value.setValueInternal(TriState::Undefined);

  return true;
}

bool InputController::addInput(Input& input)
{
  if(isInputChannel(input.channel) && isInputAddressAvailable(input.channel, input.address))
  {
    m_inputs.insert({{input.channel, input.address}, input.shared_ptr<Input>()});
    input.value.setValueInternal(TriState::Undefined);
    inputs->addObject(input.shared_ptr<Input>());
    return true;
  }
  return false;
}

bool InputController::removeInput(Input& input)
{
  assert(input.interface.value().get() == this);
  auto it = m_inputs.find({input.channel, input.address});
  if(it != m_inputs.end() && it->second.get() == &input)
  {
    m_inputs.erase(it);
    input.value.setValueInternal(TriState::Undefined);
    inputs->removeObject(input.shared_ptr<Input>());
    return true;
  }
  return false;
}

void InputController::updateInputValue(uint32_t channel, uint32_t address, TriState value)
{
  if(auto it = m_inputs.find({channel, address}); it != m_inputs.end())
    it->second->updateValue(value);
  if(auto monitor = m_inputMonitors[channel].lock())
    monitor->inputValueChanged(*monitor, address, value);
}

std::shared_ptr<InputMonitor> InputController::inputMonitor(uint32_t channel)
{
  assert(isInputChannel(channel));
  auto monitor = m_inputMonitors[channel].lock();
  if(!monitor)
  {
    monitor = std::make_shared<InputMonitor>(*this, channel);
    m_inputMonitors[channel] = monitor;
  }
  return monitor;
}

void InputController::addToWorld(InputListColumn columns)
{
  auto& object = interface();
  inputs.setValueInternal(std::make_shared<InputList>(object, inputs.name(), columns));
  object.world().inputControllers->add(std::dynamic_pointer_cast<InputController>(object.shared_from_this()));
}

void InputController::destroying()
{
  auto& object = interface();
  while(!inputs->empty())
  {
    const auto& input = inputs->front();
    assert(input->interface.value() == std::dynamic_pointer_cast<InputController>(object.shared_from_this()));
    input->interface = nullptr; // removes object form the list
  }
  object.world().inputControllers->remove(std::dynamic_pointer_cast<InputController>(object.shared_from_this()));
}

IdObject& InputController::interface()
{
  auto* object = dynamic_cast<IdObject*>(this);
  assert(object);
  return *object;
}
