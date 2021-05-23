/**
 * server/src/hardware/output/keyboard/loconetoutputkeyboard.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_OUTPUT_KEYBOARD_LOCONETOUTPUTKEYBOARD_HPP
#define TRAINTASTIC_SERVER_HARDWARE_OUTPUT_KEYBOARD_LOCONETOUTPUTKEYBOARD_HPP

#include "outputkeyboard.hpp"

namespace LocoNet {
  class LocoNet;
}

class LocoNetOutputKeyboard final : public OutputKeyboard
{
  protected:
    std::shared_ptr<LocoNet::LocoNet> m_loconet;

  public:
    CLASS_ID("output_keyboard.loconet")

    LocoNetOutputKeyboard(std::shared_ptr<LocoNet::LocoNet> loconet);
    ~LocoNetOutputKeyboard() final;

    std::string getObjectId() const final { return ""; }

    std::vector<OutputInfo> getOutputInfo() const final;
    void setOutputValue(uint32_t address, bool value) final;
};

#endif
