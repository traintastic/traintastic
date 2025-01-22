/**
 * server/src/hardware/output/keyboard/outputkeyboard.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022,2024 Reinder Feenstra
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
#include "../../../core/attributes.hpp"
#include "../../../utils/inrange.hpp"

OutputKeyboard::OutputKeyboard(OutputController& controller, OutputChannel channel_, OutputType outputType_)
  : m_controller{controller}
  , channel{this, "channel", channel_, PropertyFlags::Constant | PropertyFlags::NoStore}
  , outputType{this, "output_type", outputType_, PropertyFlags::Constant | PropertyFlags::NoStore}
  , addressMin{this, "address_min", m_controller.outputAddressMinMax(channel).first, PropertyFlags::ReadOnly | PropertyFlags::NoStore}
  , addressMax{this, "address_max", m_controller.outputAddressMinMax(channel).second, PropertyFlags::ReadOnly | PropertyFlags::NoStore}
  , outputUsedChanged(*this, "output_used_changed", EventFlags::Public)
{
  Attributes::addValues(channel, outputChannelValues);
  m_interfaceItems.add(channel);

  Attributes::addValues(outputType, outputTypeValues);
  m_interfaceItems.add(outputType);

  m_interfaceItems.add(addressMin);
  m_interfaceItems.add(addressMax);

  m_interfaceItems.add(outputUsedChanged);
}

std::string OutputKeyboard::getObjectId() const
{
  return "";
}

void OutputKeyboard::fireOutputUsedChanged(uint32_t id, bool used)
{
  fireEvent(outputUsedChanged, id, used);
}
