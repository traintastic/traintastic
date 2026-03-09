/**
 * server/src/hardware/input/monitor/inputmonitor.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2025 Reinder Feenstra
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

#include "inputmonitor.hpp"
#include "../inputcontroller.hpp"
#include "../input.hpp"
#include "../../../core/method.tpp"

InputMonitor::InputMonitor(InputController& controller, InputChannel channel)
  : m_controller{controller}
  , m_channel{channel}
  , addressMin{this, "address_min", m_controller.inputAddressMinMax(m_channel).first, PropertyFlags::ReadOnly | PropertyFlags::NoStore}
  , addressMax{this, "address_max", m_controller.inputAddressMinMax(m_channel).second, PropertyFlags::ReadOnly | PropertyFlags::NoStore}
  , simulateInputChange{*this, "simulate_input_change", MethodFlags::NoScript,
      [this](uint32_t address)
      {
        m_controller.inputSimulateChange(m_channel, address, SimulateInputAction::Toggle);
      }}
  , inputUsedChanged(*this, "input_used_changed", EventFlags::Public)
  , inputValueChanged(*this, "input_value_changed", EventFlags::Public)
{
  m_interfaceItems.add(addressMin);
  m_interfaceItems.add(addressMax);
  m_interfaceItems.add(simulateInputChange);
  m_interfaceItems.add(inputUsedChanged);
  m_interfaceItems.add(inputValueChanged);
}

std::string InputMonitor::getObjectId() const
{
  return ""; // todo
}

std::vector<InputMonitor::InputInfo> InputMonitor::getInputInfo() const
{
  std::vector<InputInfo> states;
  for(auto it : m_controller.inputMap())
  {
    const auto& input = *(it.second);
    states.emplace_back(InputInfo{input.address.value(), true, input.value.value()});
  }
  return states;
}

void InputMonitor::fireInputUsedChanged(uint32_t address, bool used)
{
  fireEvent(inputUsedChanged, address, used);
}

void InputMonitor::fireInputValueChanged(uint32_t address, TriState value)
{
  fireEvent(inputValueChanged, address, value);
}
