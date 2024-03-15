/**
 * server/src/hardware/protocol/xpressnet/settings.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021,2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_XPRESSNET_SETTINGS_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_XPRESSNET_SETTINGS_HPP

#include "../../../core/subobject.hpp"
#include "../../../core/property.hpp"
#include "../../../enum/xpressnetcommandstation.hpp"
#include "config.hpp"

namespace XpressNet {

class Settings final : public SubObject
{
  protected:
    void loaded() final;

    void commandStationChanged(XpressNetCommandStation value);
    void setCommandStationCustom();

  public:
    CLASS_ID("xpressnet_settings")

    Property<XpressNetCommandStation> commandStation;
    Property<bool> useEmergencyStopLocomotiveCommand;
    Property<bool> useRocoAccessoryAddressing;
    Property<bool> useRocoF13F20Command;
    Property<bool> debugLogInput;
    Property<bool> debugLogRXTX;

    Settings(Object& _parent, std::string_view parentPropertyName);

    Config config() const;
};

}

#endif
