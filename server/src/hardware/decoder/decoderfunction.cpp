/**
 * Traintastic
 *
 * Copyright (C) 2019-2020 Reinder Feenstra <reinderfeenstra@gmail.com>
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

const std::shared_ptr<DecoderFunction> DecoderFunction::null;

std::shared_ptr<DecoderFunction> DecoderFunction::create(Decoder& decoder, std::string_view _id)
{
  auto obj = std::make_shared<DecoderFunction>(decoder, _id);
  obj->addToWorld();
  return obj;
}

DecoderFunction::DecoderFunction(Decoder& decoder, std::string_view _id) :
  Output(decoder.world(), _id),
  m_decoder{decoder},
  number{this, "number", 0, PropertyFlags::ReadWrite | PropertyFlags::Store},
  name{this, "name", "", PropertyFlags::ReadWrite | PropertyFlags::Store}
{
  auto w = decoder.world().lock();
  const bool editable = w && contains(w->state.value(), WorldState::Edit);

  Attributes::addEnabled(number, editable);
  m_interfaceItems.add(number);
  Attributes::addEnabled(name, editable);
  m_interfaceItems.add(name);
}

void DecoderFunction::worldEvent(WorldState state, WorldEvent event)
{
  IdObject::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  number.setAttributeEnabled(editable);
  name.setAttributeEnabled(editable);
}

bool DecoderFunction::setValue(bool& value)
{
  return true;//m_decoder;
}

void DecoderFunction::valueChanged(bool)
{
  m_decoder.changed(DecoderChangeFlags::FunctionValue, number);
}
