/**
 * server/src/hardware/decoder/decodercontroller.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021 Reinder Feenstra
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
#include "../../utils/almostzero.hpp"

bool DecoderController::addDecoder(Decoder& decoder)
{
  if(decoder.protocol != DecoderProtocol::Auto && findDecoder(decoder) != m_decoders.end())
    return false;
  else if(findDecoder(DecoderProtocol::Auto, decoder.address) != m_decoders.end())
    return false;

  m_decoders.emplace_back(decoder.shared_ptr<Decoder>());
  return true;
}

bool DecoderController::removeDecoder(Decoder& decoder)
{
  auto it = findDecoder(decoder);
  if(it != m_decoders.end())
  {
    m_decoders.erase(it);
    return true;
  }
  else
    return false;
}

const std::shared_ptr<Decoder>& DecoderController::getDecoder(DecoderProtocol protocol, uint16_t address, bool dccLongAddress, bool fallbackToProtocolAuto)
{
  auto it = findDecoder(protocol, address, dccLongAddress);
  if(it != m_decoders.end())
    return *it;
  else if(fallbackToProtocolAuto && protocol != DecoderProtocol::Auto && (it = findDecoder(DecoderProtocol::Auto, address)) != m_decoders.end())
    return *it;
  else
    return Decoder::null;
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
  else
  {
    return std::find_if(m_decoders.begin(), m_decoders.end(),
      [protocol, address](const auto& it)
      {
        return it->protocol == protocol && it->address == address;
      });
  }
}

void DecoderController::restoreDecoderSpeed()
{
  for(const auto& decoder : m_decoders)
    if(!decoder->emergencyStop && !almostZero(decoder->throttle.value()))
      decoderChanged(*decoder, DecoderChangeFlags::Throttle, 0);
}
