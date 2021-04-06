/**
 * server/src/hardware/commandstation/loconetserial.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_COMMANDSTATION_LOCONETSERIAL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_COMMANDSTATION_LOCONETSERIAL_HPP

#include "serialcommandstation.hpp"
#include "../protocol/loconet/loconet.hpp"
#include "../../enum/loconetserialinterface.hpp"

class LocoNetSerial : public SerialCommandStation
{
  protected:
    void emergencyStopChanged(bool value) final;
    void powerOnChanged(bool value) final;
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber) final;

    bool start();
    bool send(const LocoNet::Message& msg);
    void started() final;
    void read() final;

  public:
    CLASS_ID("command_station.loconet_serial")
    CREATE(LocoNetSerial)

    Property<LocoNetSerialInterface> interface;
    ObjectProperty<LocoNet::LocoNet> loconet;

    LocoNetSerial(const std::weak_ptr<World>& world, std::string_view _id);
};

#endif
