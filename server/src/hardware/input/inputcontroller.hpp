/**
 * server/src/hardware/input/inputcontroller.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INPUT_INPUTCONTROLLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INPUT_INPUTCONTROLLER_HPP

#include <cstdint>
#include <unordered_map>
#include <memory>
#include "../../enum/tristate.hpp"

class Input;
class InputMonitor;

class InputController
{
  public:
    using InputMap = std::unordered_map<uint32_t, std::shared_ptr<Input>>;

  protected:
    InputMap m_inputs;
    std::weak_ptr<InputMonitor> m_inputMonitor;

  public:
    /**
     *
     */
    inline const InputMap& inputs() const { return m_inputs; }

    /**
     *
     */
    virtual std::pair<uint32_t, uint32_t> inputAddressMinMax() const = 0;

    /**
     *
     */
    [[nodiscard]] virtual bool isInputAddressAvailable(uint32_t address) const;

    /**
     * @brief Get the next unused input address
     *
     * @return An usused address or #Input::invalidAddress if no unused address is available.
     */
    uint32_t getUnusedInputAddress() const;

    /**
     *
     * @return \c true if changed, \c false otherwise.
     */
    [[nodiscard]] virtual bool changeInputAddress(Input& input, uint32_t newAddress);

    /**
     *
     * @return \c true if added, \c false otherwise.
     */
    [[nodiscard]] virtual bool addInput(Input& input);

    /**
     *
     * @return \c true if removed, \c false otherwise.
     */
    [[nodiscard]] virtual bool removeInput(Input& input);

    /**
     * @brief Update the input value
     *
     * This function should be called by the hardware layer whenever the input value changes.
     *
     * @param[in] address Input address
     * @param[in] value New input value
     */
    void updateInputValue(uint32_t address, TriState value);

    /**
     *
     *
     */
    std::shared_ptr<InputMonitor> inputMonitor();
};

#endif
