/**
 * server/src/hardware/commandstation/dccplusplusserial.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_COMMANDSTATION_dccplusplusserial_HPP
#define TRAINTASTIC_SERVER_HARDWARE_COMMANDSTATION_dccplusplusserial_HPP

#include "serialcommandstation.hpp"
#include "../protocol/dccplusplus/dccplusplus.hpp"

class DCCPlusPlusSerial : public SerialCommandStation
{
  protected:
    void emergencyStopChanged(bool value) final;
    void powerOnChanged(bool value) final;
    void checkDecoder(const Decoder& decoder) const;
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber) final;

    bool send(std::string_view message);
    void started() final;
    void read() final;

  public:
    CLASS_ID("command_station.dccplusplus_serial")
    CREATE(DCCPlusPlusSerial)

    ObjectProperty<DCCPlusPlus::DCCPlusPlus> dccPlusPlus;

    DCCPlusPlusSerial(const std::weak_ptr<World>& world, std::string_view _id);
};

#endif
