/**
 * server/src/hardware/protocol/xpressnet/xpressnetinputmonitor.cpp
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

#include "xpressnetinputmonitor.hpp"
#include "xpressnet.hpp"
#include "../../input/xpressnetinput.hpp"

XpressNetInputMonitor::XpressNetInputMonitor(std::shared_ptr<XpressNet::XpressNet> xpressnet) :
  InputMonitor(),
  m_xpressnet{std::move(xpressnet)}
{
  addressMin.setValueInternal(XpressNetInput::addressMin);
  addressMax.setValueInternal(XpressNetInput::addressMax);
  m_xpressnet->m_inputMonitors.emplace_back(this);
}

XpressNetInputMonitor::~XpressNetInputMonitor()
{
  if(auto it = std::find(m_xpressnet->m_inputMonitors.begin(), m_xpressnet->m_inputMonitors.end(), this); it != m_xpressnet->m_inputMonitors.end())
    m_xpressnet->m_inputMonitors.erase(it);
}

std::vector<InputMonitor::InputInfo> XpressNetInputMonitor::getInputInfo() const
{
  std::vector<InputInfo> inputInfo;
  for(auto it : m_xpressnet->m_inputs)
  {
    XpressNetInput& input = *(it.second);
    InputInfo info(input.address, input.id, input.value);
    inputInfo.push_back(info);
  }
  return inputInfo;
}
