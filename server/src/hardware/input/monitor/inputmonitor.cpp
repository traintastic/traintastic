/**
 * server/src/hardware/input/monitor/inputmonitor.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2023 Reinder Feenstra
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

InputMonitor::InputMonitor(InputController& controller, uint32_t channel)
  : m_controller{controller}
  , m_channel{channel}
  , addressMin{this, "address_min", m_controller.inputAddressMinMax(m_channel).first, PropertyFlags::ReadOnly | PropertyFlags::NoStore}
  , addressMax{this, "address_max", m_controller.inputAddressMinMax(m_channel).second, PropertyFlags::ReadOnly | PropertyFlags::NoStore}
  , simulateInputChange{*this, "simulate_input_change", MethodFlags::NoScript,
      [this](uint32_t address)
      {
        m_controller.inputSimulateChange(m_channel, address, SimulateInputAction::Toggle);
      }}
{
  m_interfaceItems.add(addressMin);
  m_interfaceItems.add(addressMax);
  m_interfaceItems.add(simulateInputChange);
}

std::string InputMonitor::getObjectId() const
{
  return ""; // todo
}

std::vector<InputMonitor::InputInfo> InputMonitor::getInputInfo() const
{
  std::vector<InputInfo> inputInfo;
  for(auto it : m_controller.inputMap())
  {
    const auto& input = *(it.second);
    InputInfo info(input.address, input.id, input.value);
    inputInfo.push_back(info);
  }
  return inputInfo;
}
