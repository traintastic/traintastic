/**
 * server/src/hardware/output/keyboard/outputkeyboard.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_OUTPUT_KEYBOARD_OUTPUTKEYBOARD_HPP
#define TRAINTASTIC_SERVER_HARDWARE_OUTPUT_KEYBOARD_OUTPUTKEYBOARD_HPP

#include "../../../core/object.hpp"
#include <vector>
#include <traintastic/enum/outputchannel.hpp>
#include <traintastic/enum/outputtype.hpp>
#include "../outputvalue.hpp"
#include "../../../core/property.hpp"
#include "../../../core/event.hpp"

class OutputController;

class OutputKeyboard : public Object
{
  protected:
    OutputController& m_controller;

    OutputKeyboard(OutputController& controller, OutputChannel channel_, OutputType outputType_);

  public:
    struct OutputInfo
    {
      uint32_t address;
      bool used;
      OutputValue value;
    };

    Property<OutputChannel> channel;
    Property<OutputType> outputType;
    Property<uint32_t> addressMin;
    Property<uint32_t> addressMax;
    Event<uint32_t, bool> outputUsedChanged;

    std::string getObjectId() const final;

    virtual std::vector<OutputInfo> getOutputInfo() const = 0;
    void fireOutputUsedChanged(uint32_t id, bool used);
    virtual void fireOutputValueChanged(uint32_t address, OutputValue value) = 0;
};

#endif
