/**
 * server/src/hardware/input/inputcontroller.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2025 Reinder Feenstra
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
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>
#include <traintastic/enum/inputchannel.hpp>
#include "../../core/objectproperty.hpp"
#include "../../enum/tristate.hpp"
#include "../../enum/simulateinputaction.hpp"

#ifdef interface
  #undef interface // interface is defined in combaseapi.h
#endif

class IdObject;
class Input;
class InputMonitor;

class InputList;
enum class InputListColumn;

class InputController
{
  public:
    struct InputMapKey
    {
      InputChannel channel;
      uint32_t address;

      inline bool operator ==(const InputMapKey other) const noexcept
      {
        return channel == other.channel && address == other.address;
      }
    };
    static_assert(sizeof(InputMapKey) == sizeof(uint64_t));

    struct InputMapKeyHash
    {
      std::size_t operator()(const InputMapKey& value) const noexcept
      {
        return (static_cast<uint64_t>(value.channel) << (8 * sizeof(value.address))) | value.address;
      }
    };

    using InputMap = std::unordered_map<InputMapKey, std::shared_ptr<Input>, InputMapKeyHash>;

  private:
    std::shared_ptr<InputController> shared_ptr();
    IdObject& interface();

  protected:
    InputMap m_inputs;
    std::unordered_map<InputChannel, std::weak_ptr<InputMonitor>> m_inputMonitors;

    InputController(IdObject& interface);

    void addToWorld(InputListColumn columns);
    void destroying();

  public:
    ObjectProperty<InputList> inputs;

    /**
     *
     */
    inline const InputMap& inputMap() const { return m_inputs; }

    /**
     *
     */
    virtual std::span<const InputChannel> inputChannels() const = 0;

    /**
     *
     */
    bool isInputChannel(InputChannel channel) const;

    /**
     *
     */
    virtual std::pair<uint32_t, uint32_t> inputAddressMinMax(InputChannel channel) const = 0;

    /**
     *
     */
    [[nodiscard]] virtual bool isInputAvailable(InputChannel channel, uint32_t address) const;

    /**
     * \brief Try get the lowest unused input address
     *
     * \return An usused address or `std::nullopt` if no unused address is available.
     */
    std::optional<uint32_t> getUnusedInputAddress(InputChannel channel) const;

    /**
     * \brief Get an input.
     *
     * For each channel/address combination an input object is create once,
     * if an input is requested multiple time they all share the same instance.
     * Once the object that uses the input is no longer using it,
     * it must be released using \ref releaseInput .
     * The input object will be destroyed when the are zero users.
     *
     * \param[in] channel The input channel.
     * \param[in] address The input address.
     * \param[in] usedBy The object the will use the input.
     * \return An input object if the channel/address combination is valid, \c nullptr otherwise.
     */
    std::shared_ptr<Input> getInput(InputChannel channel, uint32_t address, Object& usedBy);

    /**
     * \brief Release an input.
     *
     * \param[in] input The input to release.
     * \param[in] usedBy The object no longer using the input.
     * \see getInput
     */
    void releaseInput(Input& output, Object& usedBy);

    /**
     * @brief Update the input value
     *
     * This function should be called by the hardware layer whenever the input value changes.
     *
     * @param[in] channel Input channel
     * @param[in] address Input address
     * @param[in] value New input value
     */
    void updateInputValue(InputChannel channel, uint32_t address, TriState value);

    /**
     *
     *
     */
    std::shared_ptr<InputMonitor> inputMonitor(InputChannel channel);

    /**
     * \brief Simulate input change
     * \param[in] channel Input channel
     * \param[in] address Input address
     * \param[in] action Simulation action to perform
     */
    virtual void inputSimulateChange(InputChannel /*channel*/, uint32_t /*address*/, SimulateInputAction /*action*/) {}
};

#endif
