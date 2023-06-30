/**
 * server/src/hardware/decoder/decoderfunction.cpp
 *
 * Copyright (C) 2019-2021 Reinder Feenstra <reinderfeenstra@gmail.com>
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

#include "decoderfunction.hpp"
#include "decoder.hpp"
#include "decoderchangeflags.hpp"
#include "../../world/world.hpp"
#include "../../core/attributes.hpp"
#include "../../core/objectproperty.tpp"
#include "../../utils/displayname.hpp"
#include "../../log/logmessageexception.hpp"

const std::shared_ptr<DecoderFunction> DecoderFunction::null;

DecoderFunction::DecoderFunction(Decoder& decoder, uint8_t _number) :
  m_decoder{decoder},
  number{this, "number", _number, PropertyFlags::ReadWrite | PropertyFlags::Store, nullptr,
    [this](const uint8_t& fn)
    {
      if(m_decoder.hasFunction(fn))
        throw LogMessageException(LogMessage::E2012_FUNCTION_NUMBER_ALREADY_IN_USE);
      return true;
    }},
  name{this, "name", "", PropertyFlags::ReadWrite | PropertyFlags::Store},
  type{this, "type", DecoderFunctionType::OnOff, PropertyFlags::ReadWrite | PropertyFlags::Store,
    [this](DecoderFunctionType /*type*/)
    {
      typeChanged();
    }},
  function{this, "function", DecoderFunctionFunction::Generic, PropertyFlags::ReadWrite | PropertyFlags::Store},
  value{this, "value", false, PropertyFlags::ReadWrite | PropertyFlags::StoreState,
    [this](bool /*newValue*/)
    {
      m_decoder.changed(DecoderChangeFlags::FunctionValue, number);
    }}
{
  const bool editable = contains(decoder.world().state.value(), WorldState::Edit);

  Attributes::addEnabled(number, editable);
  m_interfaceItems.add(number);
  Attributes::addDisplayName(name, DisplayName::Object::name);
  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);
  Attributes::addEnabled(type, editable);
  Attributes::addValues(type, decoderFunctionTypeValues);
  m_interfaceItems.add(type);
  Attributes::addEnabled(function, editable);
  Attributes::addValues(function, decoderFunctionFunctionValues);
  m_interfaceItems.add(function);
  Attributes::addEnabled(value, true);
  Attributes::addObjectEditor(value, false);
  m_interfaceItems.add(value);
}

std::string DecoderFunction::getObjectId() const
{
  return m_decoder.functions->getObjectId().append(".").append(m_decoder.functions->items.name()).append(".f").append(std::to_string(number.value()));
}

void DecoderFunction::loaded()
{
  Object::loaded();
  typeChanged();
}

void DecoderFunction::worldEvent(WorldState state, WorldEvent event)
{
  Object::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(number, editable);
  Attributes::setEnabled(name, editable);
  Attributes::setEnabled(type, editable);
  Attributes::setEnabled(function, editable);
}

void DecoderFunction::typeChanged()
{
  switch(type)
  {
    case DecoderFunctionType::AlwaysOff:
      value = false;
      break;

    case DecoderFunctionType::AlwaysOn:
      value = true;
      break;

    case DecoderFunctionType::OnOff:
    case DecoderFunctionType::Hold:
    case DecoderFunctionType::Momentary:
      break;
  }
  Attributes::setEnabled(value, !isAlwaysOffOrOn(type));
}
