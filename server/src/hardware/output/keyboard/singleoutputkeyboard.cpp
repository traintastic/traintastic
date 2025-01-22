/**
 * server/src/hardware/output/keyboard/singleoutputkeyboard.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#include "singleoutputkeyboard.hpp"
#include "../singleoutput.hpp"
#include "../outputcontroller.hpp"
#include "../../../core/method.tpp"

SingleOutputKeyboard::SingleOutputKeyboard(OutputController& controller, OutputChannel channel_)
  : OutputKeyboard(controller, channel_, OutputType::Single)
  , setOutputValue(*this, "set_output_value",
      [this](uint32_t address, bool value)
      {
        return m_controller.setOutputValue(channel, address, toTriState(value));
      })
  , outputValueChanged(*this, "output_value_changed", EventFlags::Public)
{
  m_interfaceItems.add(setOutputValue);
  m_interfaceItems.add(outputValueChanged);
}

std::vector<OutputKeyboard::OutputInfo> SingleOutputKeyboard::getOutputInfo() const
{
  std::vector<OutputInfo> states;
  for(const auto& it : m_controller.outputMap())
  {
    if(it.second->channel == channel)
    {
      const auto& output = static_cast<const SingleOutput&>(*it.second);
      states.emplace_back(OutputInfo{output.address.value(), true, output.value.value()});
    }
  }
  return states;
}

void SingleOutputKeyboard::fireOutputValueChanged(uint32_t address, OutputValue value)
{
  fireEvent(outputValueChanged, address, std::get<TriState>(value));
}
