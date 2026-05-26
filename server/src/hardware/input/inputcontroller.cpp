/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2021-2026 Reinder Feenstra
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
#include "../../utils/contains.hpp"
#include "../../utils/displayname.hpp"
#include "../../utils/inrange.hpp"
#include "../../world/world.hpp"
#include "../../world/getworld.hpp"

InputController::InputController(IdObject& interface)
  : inputs{&interface, "inputs", nullptr, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::SubObject}
{
  Attributes::addDisplayName(inputs, DisplayName::Hardware::inputs);
}

bool InputController::isInputChannel(InputChannel channel) const
{
  assert(!inputChannels().empty());
  return contains(inputChannels(), channel);
}

bool InputController::isInputLocation(InputChannel channel, const InputLocation& location) const
{
  if(hasAddressLocation(channel))
  {
    return inRange(std::get<InputAddress>(location).address, inputAddressMinMax(channel));
  }
  return false;
}

bool InputController::isInputAvailable(InputChannel channel, const InputLocation& location) const
{
  assert(isInputChannel(channel));
  return
    isInputLocation(channel, location) &&
    m_inputs.find({channel, location}) == m_inputs.end();
}

std::optional<uint32_t> InputController::getUnusedInputAddress(InputChannel channel) const
{
  assert(isInputChannel(channel));
  if(hasAddressLocation(channel))
  {
    const auto end = m_inputs.cend();
    const auto range = inputAddressMinMax(channel);
    for(uint32_t address = range.first; address < range.second; address++)
    {
      if(m_inputs.find({channel, InputAddress(address)}) == end)
      {
        return address;
      }
    }
  }
  return std::nullopt;
}

std::shared_ptr<Input> InputController::getInput(InputChannel channel, const InputLocation& location, Object& usedBy)
{
  if(!isInputChannel(channel) || !isInputLocation(channel, location))
  {
    return {};
  }

  // Check if already exists:
  if(auto it = m_inputs.find({channel, location}); it != m_inputs.end())
  {
    it->second->m_usedBy.emplace(usedBy.shared_from_this());
    return it->second;
  }

  // Create new input:
  std::shared_ptr<Input> input;
  if(hasAddressLocation(channel))
  {
    assert(std::holds_alternative<InputAddress>(location));
    input = std::make_shared<Input>(shared_ptr(), channel, std::nullopt, std::get<InputAddress>(location).address);
  }
  else if(hasNodeAddressLocation(channel))
  {
    assert(std::holds_alternative<InputNodeAddress>(location));
    input = std::make_shared<Input>(shared_ptr(), channel, std::get<InputNodeAddress>(location).node, std::get<InputNodeAddress>(location).address);
  }
  else [[unlikely]]
  {
    assert(false);
    return {};
  }
  input->m_usedBy.emplace(usedBy.shared_from_this());
  m_inputs.emplace(InputMapKey{channel, location}, input);
  inputs->addObject(input);
  getWorld(inputs.object()).inputs->addObject(input);

  if(auto monitor = m_inputMonitors[channel].lock(); monitor && hasAddressLocation(channel))
  {
    monitor->fireInputUsedChanged(input->address, true);
  }

  return input;
}

void InputController::releaseInput(Input& input, Object& usedBy)
{
  auto inputShared = input.shared_ptr<Input>();
  input.m_usedBy.erase(usedBy.shared_from_this());
  if(input.m_usedBy.empty())
  {
    const auto channel = input.channel.value();
    const auto location = input.location();

    m_inputs.erase({channel, location});
    inputs->removeObject(inputShared);
    getWorld(inputs.object()).inputs->removeObject(inputShared);
    inputShared->destroy();
    inputShared.reset();

    if(auto monitor = m_inputMonitors[channel].lock(); monitor && hasAddressLocation(channel))
    {
      monitor->fireInputUsedChanged(std::get<InputAddress>(location).address, false);
    }
  }
}

void InputController::updateInputValue(InputChannel channel, const InputLocation& location, TriState value)
{
  if(auto it = m_inputs.find({channel, location}); it != m_inputs.end())
  {
    it->second->updateValue(value);
  }
  if(auto monitor = m_inputMonitors[channel].lock(); monitor && std::holds_alternative<InputAddress>(location))
  {
    monitor->fireInputValueChanged(std::get<InputAddress>(location).address, value);
  }
}

std::shared_ptr<InputMonitor> InputController::inputMonitor(InputChannel channel)
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
    input->interface.setValueInternal(nullptr);
    input->destroy(); // notify consumers we're dying
    inputs->removeObject(input);
  }
  object.world().inputControllers->remove(std::dynamic_pointer_cast<InputController>(object.shared_from_this()));
}

IdObject& InputController::interface()
{
  auto* object = dynamic_cast<IdObject*>(this);
  assert(object);
  return *object;
}

std::shared_ptr<InputController> InputController::shared_ptr()
{
  auto self = std::dynamic_pointer_cast<InputController>(inputs.object().shared_from_this());
  assert(self);
  return self;
}
