/**
 * server/src/hardware/protocol/loconet/loconetoutputkeyboard.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
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

#include "loconetoutputkeyboard.hpp"
#include "loconet.hpp"
#include "../../output/loconetoutput.hpp"

LocoNetOutputKeyboard::LocoNetOutputKeyboard(std::shared_ptr<LocoNet::LocoNet> loconet) :
  OutputKeyboard(),
  m_loconet{std::move(loconet)}
{
  addressMin.setValueInternal(LocoNetOutput::addressMin);
  addressMax.setValueInternal(LocoNetOutput::addressMax);
  m_loconet->m_outputKeyboards.emplace_back(this);
}

LocoNetOutputKeyboard::~LocoNetOutputKeyboard()
{
  if(auto it = std::find(m_loconet->m_outputKeyboards.begin(), m_loconet->m_outputKeyboards.end(), this); it != m_loconet->m_outputKeyboards.end())
    m_loconet->m_outputKeyboards.erase(it);
}

std::vector<OutputKeyboard::OutputInfo> LocoNetOutputKeyboard::getOutputInfo() const
{
  std::vector<OutputInfo> outputInfo;
  for(auto it : m_loconet->m_outputs)
  {
    LocoNetOutput& output = *(it.second);
    OutputInfo info(output.address, output.id, output.value);
    outputInfo.push_back(info);
  }
  return outputInfo;
}

void LocoNetOutputKeyboard::setOutputValue(uint32_t address, bool value)
{
  if(address < LocoNetOutput::addressMin || address > LocoNetOutput::addressMax)
    return;

  auto it = m_loconet->m_outputs.find(address);
  if(it != m_loconet->m_outputs.end())
    it->second->value = toTriState(value);
  else
    m_loconet->send(LocoNet::SwitchRequest(address - 1, value));
}
