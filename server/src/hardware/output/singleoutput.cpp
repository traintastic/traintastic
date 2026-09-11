/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2024-2026 Reinder Feenstra
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

#include "singleoutput.hpp"
#include "outputcontroller.hpp"
#include "../../core/attributes.hpp"
#include "../../core/method.tpp"
#include "../../core/objectproperty.tpp"

SingleOutput::SingleOutput(std::shared_ptr<OutputController> outputController, OutputChannel channel_, std::optional<uint32_t> node_, uint32_t address_)
  : AddressOutput(std::move(outputController), channel_, OutputType::Pair, node_, address_)
  , value{this, "value", TriState::Undefined, PropertyFlags::ReadOnly | PropertyFlags::StoreState | PropertyFlags::ScriptReadOnly}
  , setValue{*this, "set_value", MethodFlags::ScriptCallable,
      [this](bool newValue)
      {
        if(interface)
        {
          if(hasNode)
          {
            return interface->setOutputValue(channel, OutputNodeAddress(node, address), toTriState(newValue));
          }
          return interface->setOutputValue(channel, OutputAddress(address), toTriState(newValue));
        }
        return false;
      }}
  , onValueChanged{*this, "on_value_changed", EventFlags::Scriptable}
{
  Attributes::addObjectEditor(value, false);
  Attributes::addValues(value, TriStateValues);
  m_interfaceItems.add(value);

  Attributes::addObjectEditor(setValue, false);
  m_interfaceItems.add(setValue);

  m_interfaceItems.add(onValueChanged);
}

void SingleOutput::updateValue(TriState _value)
{
  value.setValueInternal(_value);
  if(value != TriState::Undefined)
  {
    fireEvent(onValueChanged, value == TriState::True, shared_ptr<SingleOutput>());
    fireEvent(onValueChangedGeneric, shared_ptr<Output>());
  }
}
