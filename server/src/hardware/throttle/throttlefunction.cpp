/**
 * server/src/hardware/throttle/throttlefunction.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#include "throttlefunction.hpp"
#include "throttle.hpp"
#include "../decoder/decoder.hpp"

ThrottleFunction::ThrottleFunction(Throttle& throttle, uint32_t number_)
  : m_throttle{throttle}
  , number{this, "number", number_, PropertyFlags::ReadOnly}
  , name{this, "name", {}, PropertyFlags::ReadOnly}
  , value{this, "value", false, PropertyFlags::ReadWrite}
  , press{*this, "press",
      [this]()
      {
        if(!m_throttle.m_decoder)
          return;

        if(auto function = m_throttle.m_decoder->getFunction(number))
        {
          switch(function->type.value())
          {
            case DecoderFunctionType::Hold:
            case DecoderFunctionType::Momentary:
              function->value = false;
              break;

            case DecoderFunctionType::OnOff:
              // toggle when button is pushed, do nothing on release
              function->value = !function->value;
              break;

            case DecoderFunctionType::AlwaysOff:
            case DecoderFunctionType::AlwaysOn:
              break; // do nothing
          }
        }
      }}
  , release{*this, "release",
      [this]()
      {
        if(!m_throttle.m_decoder)
          return;

        if(auto function = m_throttle.m_decoder->getFunction(number))
        {
          switch(function->type.value())
          {
            case DecoderFunctionType::Hold:
            case DecoderFunctionType::Momentary:
              function->value = false;
              break;

            case DecoderFunctionType::OnOff:
            case DecoderFunctionType::AlwaysOff:
            case DecoderFunctionType::AlwaysOn:
              break; // do nothing
          }
        }
    }}
{
  if(const auto& decoder = m_throttle.m_decoder)
  {
    if(auto function = decoder->getFunction(number))
    {
      name.setValueInternal(function->name);
      value.setValueInternal(function->value);
    }
  }
}

std::string ThrottleFunction::getObjectId() const
{
  return m_throttle.getObjectId().append(".").append(m_throttle.functions.name()).append(".f").append(std::to_string(number));
}
