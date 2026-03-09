/**
 * server/src/hardware/input/monitor/inputmonitor.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2025 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INPUT_MONITOR_INPUTMONITOR_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INPUT_MONITOR_INPUTMONITOR_HPP

#include "../../../core/object.hpp"
#include <vector>
#include <traintastic/enum/inputchannel.hpp>
#include "../../../core/property.hpp"
#include "../../../core/method.hpp"
#include "../../../core/event.hpp"
#include "../../../enum/tristate.hpp"

class InputController;

class InputMonitor : public Object
{
  CLASS_ID("input_monitor")

  private:
    InputController& m_controller;
    const InputChannel m_channel;

  public:
    struct InputInfo
    {
      uint32_t address;
      bool used;
      TriState value;
    };

    Property<uint32_t> addressMin;
    Property<uint32_t> addressMax;
    Method<void(uint32_t)> simulateInputChange;
    Event<uint32_t, bool> inputUsedChanged;
    Event<uint32_t, TriState> inputValueChanged;

    InputMonitor(InputController& controller, InputChannel channel);

    std::string getObjectId() const final;

    virtual std::vector<InputInfo> getInputInfo() const;
    void fireInputUsedChanged(uint32_t address, bool used);
    void fireInputValueChanged(uint32_t address, TriState value);
};

#endif
