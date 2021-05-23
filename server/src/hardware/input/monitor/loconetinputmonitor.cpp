/**
 * server/src/hardware/input/monitor/loconetinputmonitor.cpp
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

#include "loconetinputmonitor.hpp"
#include "../../protocol/loconet/loconet.hpp"
#include "../loconetinput.hpp"

LocoNetInputMonitor::LocoNetInputMonitor(std::shared_ptr<LocoNet::LocoNet> loconet) :
  InputMonitor(),
  m_loconet{std::move(loconet)}
{
  addressMin.setValueInternal(LocoNetInput::addressMin);
  addressMax.setValueInternal(LocoNetInput::addressMax);
  m_loconet->m_inputMonitors.emplace_back(this);
}

LocoNetInputMonitor::~LocoNetInputMonitor()
{
  if(auto it = std::find(m_loconet->m_inputMonitors.begin(), m_loconet->m_inputMonitors.end(), this); it != m_loconet->m_inputMonitors.end())
    m_loconet->m_inputMonitors.erase(it);
}

std::vector<InputMonitor::InputInfo> LocoNetInputMonitor::getInputInfo() const
{
  std::vector<InputInfo> inputInfo;
  for(auto it : m_loconet->m_inputs)
  {
    LocoNetInput& input = *(it.second);
    InputInfo info(input.address, input.id, input.value);
    inputInfo.push_back(info);
  }
  return inputInfo;
}
