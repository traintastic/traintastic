/**
 * server/src/hardware/output/pairoutput.cpp
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

#include "pairoutput.hpp"
#include "outputcontroller.hpp"
#include "../../core/attributes.hpp"
#include "../../core/method.tpp"
#include "../../core/objectproperty.tpp"

PairOutput::PairOutput(std::shared_ptr<OutputController> outputController, OutputChannel channel_, uint32_t address_)
  : AddressOutput(std::move(outputController), channel_, OutputType::Pair, address_)
  , value{this, "value", OutputPairValue::Undefined, PropertyFlags::ReadOnly | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly}
  , setValue{*this, "set_value", MethodFlags::ScriptCallable,
      [this](OutputPairValue newValue)
      {
        return
          (newValue != OutputPairValue::Undefined) &&
          interface &&
          interface->setOutputValue(channel, address, newValue);
      }}
  , onValueChanged{*this, "on_value_changed", EventFlags::Scriptable}
{
  Attributes::addObjectEditor(value, false);
  Attributes::addValues(value, outputPairValueValues);
  m_interfaceItems.add(value);

  Attributes::addObjectEditor(setValue, false);
  m_interfaceItems.add(setValue);

  m_interfaceItems.add(onValueChanged);
}

void PairOutput::updateValue(OutputPairValue newValue)
{
  value.setValueInternal(newValue);
  if(newValue != OutputPairValue::Undefined)
  {
    fireEvent(onValueChanged, newValue, shared_ptr<PairOutput>());
    fireEvent(onValueChangedGeneric, shared_ptr<Output>());
  }
}
