/**
 * server/src/hardware/input/inputcontroller.cpp
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

#include "inputcontroller.hpp"
#include "input.hpp"
#include "monitor/inputmonitor.hpp"
#include "../../utils/inrange.hpp"

bool InputController::isInputAddressAvailable(uint32_t address) const
{
  return
    inRange(address, inputAddressMinMax()) &&
    m_inputs.find(address) == m_inputs.end();
}

uint32_t InputController::getUnusedInputAddress() const
{
  const auto end = m_inputs.cend();
  const auto range = inputAddressMinMax();
  for(uint32_t address = range.first; address < range.second; address++)
    if(m_inputs.find(address) == end)
      return address;
  return Input::invalidAddress;
}

bool InputController::changeInputAddress(Input& input, uint32_t newAddress)
{
  assert(input.interface.value().get() == this);

  if(!isInputAddressAvailable(newAddress))
    return false;

  auto node = m_inputs.extract(input.address); // old address
  node.key() = newAddress;
  m_inputs.insert(std::move(node));
  input.value.setValueInternal(TriState::Undefined);

  return true;
}

bool InputController::addInput(Input& input)
{
  if(isInputAddressAvailable(input.address))
  {
    m_inputs.insert({input.address, input.shared_ptr<Input>()});
    input.value.setValueInternal(TriState::Undefined);
    return true;
  }
  else
    return false;
}

bool InputController::removeInput(Input& input)
{
  assert(input.interface.value().get() == this);
  auto it = m_inputs.find(input.address);
  if(it != m_inputs.end() && it->second.get() == &input)
  {
    m_inputs.erase(it);
    input.value.setValueInternal(TriState::Undefined);
    return true;
  }
  else
    return false;
}

void InputController::updateInputValue(uint32_t address, TriState value)
{
  if(auto it = m_inputs.find(address); it != m_inputs.end())
    it->second->updateValue(value);
  if(auto monitor = m_inputMonitor.lock())
    monitor->inputValueChanged(*monitor, address, value);
}

std::shared_ptr<InputMonitor> InputController::inputMonitor()
{
  auto monitor = m_inputMonitor.lock();
  if(!monitor)
  {
    monitor = std::make_shared<InputMonitor>(*this);
    m_inputMonitor = monitor;
  }
  return monitor;
}
