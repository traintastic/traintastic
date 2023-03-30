/**
 * server/src/hardware/decoder/decodercontroller.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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

#include "decodercontroller.hpp"
#include "decoder.hpp"
#include "decoderchangeflags.hpp"
#include "list/decoderlist.hpp"
#include "list/decoderlisttablemodel.hpp"
#include "../../core/attributes.hpp"
#include "../../core/objectproperty.tpp"
#include "../../utils/displayname.hpp"
#include "../../utils/almostzero.hpp"
#include "../../world/world.hpp"

DecoderController::DecoderController(IdObject& interface, DecoderListColumn columns)
  : decoders{&interface, "decoders", nullptr, PropertyFlags::ReadOnly | PropertyFlags::NoStore | PropertyFlags::SubObject}
{
  decoders.setValueInternal(std::make_shared<DecoderList>(interface, decoders.name(), columns));

  Attributes::addDisplayName(decoders, DisplayName::Hardware::decoders);
}

bool DecoderController::addDecoder(Decoder& decoder)
{
  if(decoder.protocol != DecoderProtocol::Auto && findDecoder(decoder) != m_decoders.end())
    return false;
  if(findDecoder(DecoderProtocol::Auto, decoder.address) != m_decoders.end())
    return false;

  m_decoders.emplace_back(decoder.shared_ptr<Decoder>());
  decoders->addObject(decoder.shared_ptr<Decoder>());
  return true;
}

bool DecoderController::removeDecoder(Decoder& decoder)
{
  auto it = findDecoder(decoder);
  if(it != m_decoders.end())
  {
    m_decoders.erase(it);
    decoders->removeObject(decoder.shared_ptr<Decoder>());
    return true;
  }
  return false;
}

const std::shared_ptr<Decoder>& DecoderController::getDecoder(DecoderProtocol protocol, uint16_t address, bool dccLongAddress, bool fallbackToProtocolAuto)
{
  auto it = findDecoder(protocol, address, dccLongAddress);
  if(it != m_decoders.end())
    return *it;
  if(fallbackToProtocolAuto && protocol != DecoderProtocol::Auto && (it = findDecoder(DecoderProtocol::Auto, address)) != m_decoders.end())
    return *it;

  return Decoder::null;
}

void DecoderController::addToWorld()
{
  auto& object = interface();
  object.world().decoderControllers->add(std::dynamic_pointer_cast<DecoderController>(object.shared_from_this()));
}

void DecoderController::destroying()
{
  auto& object = interface();
  while(!decoders->empty())
  {
    const auto& decoder = decoders->front();
    assert(decoder->interface.value() == std::dynamic_pointer_cast<DecoderController>(object.shared_from_this()));
    decoder->interface = nullptr; // removes object form the list
  }

  object.world().decoderControllers->remove(std::dynamic_pointer_cast<DecoderController>(object.shared_from_this()));
}

DecoderController::DecoderVector::iterator DecoderController::findDecoder(const Decoder& decoder)
{
  return findDecoder(decoder.protocol, decoder.address, decoder.longAddress);
}

DecoderController::DecoderVector::iterator DecoderController::findDecoder(DecoderProtocol protocol, uint16_t address, bool dccLongAddress)
{
  if(protocol == DecoderProtocol::DCC)
  {
    return std::find_if(m_decoders.begin(), m_decoders.end(),
      [address, dccLongAddress](const auto& it)
      {
        return it->protocol == DecoderProtocol::DCC && it->address == address && it->longAddress == dccLongAddress;
      });
  }

  return std::find_if(m_decoders.begin(), m_decoders.end(),
    [protocol, address](const auto& it)
    {
      return it->protocol == protocol && it->address == address;
    });
}

void DecoderController::restoreDecoderSpeed()
{
  for(const auto& decoder : m_decoders)
    if(!decoder->emergencyStop && !almostZero(decoder->throttle.value()))
      decoderChanged(*decoder, DecoderChangeFlags::Throttle, 0);
}

IdObject& DecoderController::interface()
{
  auto* object = dynamic_cast<IdObject*>(this);
  assert(object);
  return *object;
}
