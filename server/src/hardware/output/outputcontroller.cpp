/**
 * server/src/hardware/output/outputcontroller.cpp
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

#include "outputcontroller.hpp"
#include "output.hpp"
#include "keyboard/outputkeyboard.hpp"
#include "../../utils/inrange.hpp"

bool OutputController::isOutputAddressAvailable(uint32_t address) const
{
  return
    inRange(address, outputAddressMinMax()) &&
    m_outputs.find(address) == m_outputs.end();
}

uint32_t OutputController::getUnusedOutputAddress() const
{
  const auto end = m_outputs.cend();
  const auto range = outputAddressMinMax();
  for(uint32_t address = range.first; address < range.second; address++)
    if(m_outputs.find(address) == end)
      return address;
  return Output::invalidAddress;
}

bool OutputController::changeOutputAddress(Output& output, uint32_t newAddress)
{
  assert(output.interface.value().get() == this);

  if(!isOutputAddressAvailable(newAddress))
    return false;

  auto node = m_outputs.extract(output.address); // old address
  node.key() = newAddress;
  m_outputs.insert(std::move(node));
  output.value.setValueInternal(TriState::Undefined);

  return true;
}

bool OutputController::addOutput(Output& output)
{
  if(isOutputAddressAvailable(output.address))
  {
    m_outputs.insert({output.address, output.shared_ptr<Output>()});
    output.value.setValueInternal(TriState::Undefined);
    return true;
  }
  else
    return false;
}

bool OutputController::removeOutput(Output& output)
{
  assert(output.interface.value().get() == this);
  auto it = m_outputs.find(output.address);
  if(it != m_outputs.end() && it->second.get() == &output)
  {
    m_outputs.erase(it);
    output.value.setValueInternal(TriState::Undefined);
    return true;
  }
  else
    return false;
}

void OutputController::updateOutputValue(uint32_t address, TriState value)
{
  if(auto it = m_outputs.find(address); it != m_outputs.end())
    it->second->updateValue(value);
  if(auto keyboard = m_outputKeyboard.lock())
    keyboard->outputValueChanged(*keyboard, address, value);
}

std::shared_ptr<OutputKeyboard> OutputController::outputKeyboard()
{
  auto keyboard = m_outputKeyboard.lock();
  if(!keyboard)
  {
    keyboard = std::make_shared<OutputKeyboard>(*this);
    m_outputKeyboard = keyboard;
  }
  return keyboard;
}
