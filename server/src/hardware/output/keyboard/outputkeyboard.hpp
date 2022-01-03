/**
 * server/src/hardware/output/keyboard/outputkeyboard.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_OUTPUT_KEYBOARD_OUTPUTKEYBOARD_HPP
#define TRAINTASTIC_SERVER_HARDWARE_OUTPUT_KEYBOARD_OUTPUTKEYBOARD_HPP

#include "../../../core/object.hpp"
#include <vector>
#include "../../../core/property.hpp"
#include "../../../enum/tristate.hpp"

class OutputController;

class OutputKeyboard : public Object
{
  CLASS_ID("output_keyboard")

  private:
    OutputController& m_controller;

  public:
    boost::signals2::signal<void(OutputKeyboard&, uint32_t, std::string_view)> outputIdChanged;
    boost::signals2::signal<void(OutputKeyboard&, uint32_t, TriState)> outputValueChanged;

    struct OutputInfo
    {
      uint32_t address;
      std::string id;
      TriState value;

      OutputInfo(uint32_t _address, std::string _id, TriState _value) :
        address{_address},
        id{std::move(_id)},
        value{_value}
      {
      }
    };

    Property<uint32_t> addressMin;
    Property<uint32_t> addressMax;

    OutputKeyboard(OutputController& controller);

    std::string getObjectId() const final;

    std::vector<OutputInfo> getOutputInfo() const;
    void setOutputValue(uint32_t address, bool value);
};

#endif
