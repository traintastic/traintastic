/**
 * server/src/hardware/protocol/dccplusplus/settings.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021,2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DCCPLUSPLUS_SETTINGS_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DCCPLUSPLUS_SETTINGS_HPP

#include "../../../core/subobject.hpp"
#include "../../../core/property.hpp"
#include "config.hpp"

namespace DCCPlusPlus {

class Settings : public SubObject
{
  CLASS_ID("dccplusplus_settings")

  private:
    static constexpr uint16_t startupDelayMin = 0;
    static constexpr uint16_t startupDelayDefault = 2'500;
    static constexpr uint16_t startupDelayMax = 60'000;

  public:
    static constexpr std::array<uint8_t, 2> speedStepValues{28, 128};

    Property<bool> useEx;
    Property<uint8_t> speedSteps;
    Property<uint16_t> startupDelay;
    Property<bool> debugLogRXTX;

    Settings(Object& _parent, std::string_view parentPropertyName);

    Config config() const;
};

}

#endif
