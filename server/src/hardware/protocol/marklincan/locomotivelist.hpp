/**
 * server/src/hardware/protocol/marklincan/locomotivelist.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLINCAN_LOCOMOTIVELIST_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_MARKLINCAN_LOCOMOTIVELIST_HPP

#include <string_view>
#include <vector>
#include <memory>
#include <traintastic/enum/decoderprotocol.hpp>
#include <traintastic/enum/decoderfunctiontype.hpp>
#include <traintastic/enum/decoderfunctionfunction.hpp>

namespace MarklinCAN {

class LocomotiveList
{
  public:
    struct Function
    {
      uint8_t nr;
      DecoderFunctionType type = DecoderFunctionType::OnOff;
      DecoderFunctionFunction function = DecoderFunctionFunction::Generic;
    };

    struct Locomotive
    {
      std::string name;
      uint16_t address = 0;
      DecoderProtocol protocol = DecoderProtocol::None;
      uint16_t sid = 0;
      uint32_t mfxUID = 0;
      std::vector<Function> functions;
    };

  private:
    std::vector<Locomotive> m_locomotives;

  public:
    LocomotiveList(std::string_view list = {});
};

}

#endif
