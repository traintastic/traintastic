/**
 * server/src/hardware/protocol/loconet/settings.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_SETTINGS_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_LOCONET_SETTINGS_HPP

#include "../../../core/subobject.hpp"
#include <traintastic/enum/loconetcommandstation.hpp>
#include "../../../core/property.hpp"
#include "config.hpp"

namespace LocoNet {

class Settings final : public SubObject
{
  CLASS_ID("loconet_settings")

  private:
    void commandStationChanged(LocoNetCommandStation value);

  protected:
    void loaded() final;

  public:
    Property<LocoNetCommandStation> commandStation;
    Property<uint16_t> echoTimeout;
    Property<uint16_t> responseTimeout;
    Property<uint8_t> locomotiveSlots;
    Property<LocoNetF9F28> f9f28;
    Property<LocoNetFastClock> fastClock;
    Property<bool> fastClockSyncEnabled;
    Property<uint8_t> fastClockSyncInterval; //!< Fast clock sync interval in seconds
    Property<bool> debugLogInput;
    Property<bool> debugLogOutput;
    Property<bool> debugLogRXTX;

    Settings(Object& _parent, std::string_view parentPropertyName);

    Config config() const;
};

}

#endif
