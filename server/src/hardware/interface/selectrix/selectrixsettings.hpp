/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2023-2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_SELECTRIX_SELECTRIXSETTINGS_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_SELECTRIX_SELECTRIXSETTINGS_HPP

#include "../../../core/subobject.hpp"
#include "../../../core/property.hpp"
#include "../../protocol/selectrix/selectrixconfig.hpp"

class SelectrixSettings final : public SubObject
{
  CLASS_ID("selectrix_settings")

  private:
    void updateVisible();

  protected:
    void loaded() final;

  public:
    Property<bool> useRautenhausCommandFormat;
    Property<uint16_t> trackPowerPollInterval;
    Property<uint16_t> locomotivePollInterval;
    Property<uint16_t> feedbackPollInterval;
    Property<uint16_t> accessoryPollInterval;
    Property<bool> debugLogRXTX;

    SelectrixSettings(Object& _parent, std::string_view parentPropertyName);

    Selectrix::Config config() const;

    void updateEnabled(bool worldEdit, bool interfaceOnline);
};

#endif
