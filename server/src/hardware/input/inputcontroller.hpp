/**
 * server/src/hardware/input/inputcontroller.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2023 Reinder Feenstra
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
#include "../../core/objectproperty.hpp"
#include "../../enum/tristate.hpp"

#ifdef interface
  #undef interface // interface is defined in combaseapi.h
#endif

class IdObject;
class Input;
class InputMonitor;

class InputList;
enum class InputListColumn;

enum class SimulateInputAction
{
  SetFalse,
  SetTrue,
  Toggle,
};

class InputController
{
  public:
    struct InputMapKey
    {
      uint32_t channel;
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
        return std::hash<uint64_t>{}(*reinterpret_cast<const uint64_t*>(&value));
      }
    };

    using InputMap = std::unordered_map<InputMapKey, std::shared_ptr<Input>, InputMapKeyHash>;

  private:
    IdObject& interface();

  protected:
    InputMap m_inputs;
    std::unordered_map<uint32_t, std::weak_ptr<InputMonitor>> m_inputMonitors;

    InputController(IdObject& interface);

    void addToWorld(InputListColumn columns);
    void destroying();

  public:
    static constexpr std::vector<uint32_t>* noInputChannels = nullptr;
    static constexpr uint32_t defaultInputChannel = 0;

    ObjectProperty<InputList> inputs;

    /**
     *
     */
    inline const InputMap& inputMap() const { return m_inputs; }

    /**
     *
     */
    virtual const std::vector<uint32_t>* inputChannels() const { return noInputChannels; }

    /**
     *
     */
    virtual const std::vector<std::string_view>* inputChannelNames() const { return nullptr; }

    /**
     *
     */
    bool isInputChannel(uint32_t channel) const;

    /**
     *
     */
    virtual std::pair<uint32_t, uint32_t> inputAddressMinMax(uint32_t channel) const = 0;

    /**
     *
     */
    [[nodiscard]] virtual bool isInputAddressAvailable(uint32_t channel, uint32_t address) const;

    /**
     * @brief Get the next unused input address
     *
     * @return An usused address or #Input::invalidAddress if no unused address is available.
     */
    uint32_t getUnusedInputAddress(uint32_t channel) const;

    /**
     *
     * @return \c true if changed, \c false otherwise.
     */
    [[nodiscard]] virtual bool changeInputChannelAddress(Input& input, uint32_t newChannel, uint32_t newAddress);

    /**
     *
     * @return \c true if added, \c false otherwise.
     */
    [[nodiscard]] bool addInput(Input& input);

    /**
     *
     * @return \c true if removed, \c false otherwise.
     */
    [[nodiscard]] bool removeInput(Input& input);

    /**
     * @brief Update the input value
     *
     * This function should be called by the hardware layer whenever the input value changes.
     *
     * @param[in] channel Input channel
     * @param[in] address Input address
     * @param[in] value New input value
     */
    void updateInputValue(uint32_t channel, uint32_t address, TriState value);

    /**
     *
     *
     */
    std::shared_ptr<InputMonitor> inputMonitor(uint32_t channel);

    /**
     * \brief Simulate input change
     * \param[in] channel Input channel
     * \param[in] address Input address
     * \param[in] action Simulation action to perform
     */
    virtual void inputSimulateChange(uint32_t /*channel*/, uint32_t /*address*/, SimulateInputAction /*action*/) {}
};

#endif
