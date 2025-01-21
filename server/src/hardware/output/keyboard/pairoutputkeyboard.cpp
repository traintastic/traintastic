/**
 * server/src/hardware/output/keyboard/pairoutputkeyboard.cpp
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

#include "pairoutputkeyboard.hpp"
#include "../pairoutput.hpp"
#include "../outputcontroller.hpp"
#include "../../../core/method.tpp"

PairOutputKeyboard::PairOutputKeyboard(OutputController& controller, OutputChannel channel_)
  : OutputKeyboard(controller, channel_, OutputType::Pair)
  , setOutputValue(*this, "set_output_value",
      [this](uint32_t address, OutputPairValue value)
      {
        return m_controller.setOutputValue(channel, address, value);
      })
  , outputValueChanged(*this, "output_value_changed", EventFlags::Public)
{
  m_interfaceItems.add(setOutputValue);
  m_interfaceItems.add(outputValueChanged);
}

std::vector<OutputKeyboard::OutputInfo> PairOutputKeyboard::getOutputInfo() const
{
  std::vector<OutputInfo> states;
  for(const auto& it : m_controller.outputMap())
  {
    if(it.second->channel == channel)
    {
      const auto& output = static_cast<const PairOutput&>(*it.second);
      states.emplace_back(OutputInfo{output.address.value(), true, output.value.value()});
    }
  }
  return states;
}

void PairOutputKeyboard::fireOutputValueChanged(uint32_t address, OutputValue value)
{
  fireEvent(outputValueChanged, address, std::get<OutputPairValue>(value));
}
