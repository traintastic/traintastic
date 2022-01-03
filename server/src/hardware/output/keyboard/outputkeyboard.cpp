/**
 * server/src/hardware/output/keyboard/outputkeyboard.cpp
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

#include "outputkeyboard.hpp"
#include "../output.hpp"
#include "../outputcontroller.hpp"
#include "../../../utils/inrange.hpp"

OutputKeyboard::OutputKeyboard(OutputController& controller)
  : m_controller{controller}
  , addressMin{this, "address_min", m_controller.outputAddressMinMax().first, PropertyFlags::ReadOnly | PropertyFlags::NoStore}
  , addressMax{this, "address_max", m_controller.outputAddressMinMax().second, PropertyFlags::ReadOnly | PropertyFlags::NoStore}
{
}

std::string OutputKeyboard::getObjectId() const
{
  return "";
}

std::vector<OutputKeyboard::OutputInfo> OutputKeyboard::getOutputInfo() const
{
  std::vector<OutputInfo> outputInfo;
  for(auto it : m_controller.outputs())
  {
    const auto& output = *(it.second);
    outputInfo.emplace_back(OutputInfo{output.address, output.id, output.value});
  }
  return outputInfo;
}

bool OutputKeyboard::setOutputValue(uint32_t address, bool value)
{
  return m_controller.setOutputValue(address, value);
}
