/**
 * server/src/hardware/output/outputcontroller.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_OUTPUT_OUTPUTCONTROLLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_OUTPUT_OUTPUTCONTROLLER_HPP

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
class Output;
class OutputKeyboard;
class OutputList;
enum class OutputListColumn;

class OutputController
{
  public:
    struct OutputMapKey
    {
      uint32_t channel;
      uint32_t address;

      inline bool operator ==(const OutputMapKey other) const noexcept
      {
        return channel == other.channel && address == other.address;
      }
    };
    static_assert(sizeof(OutputMapKey) == sizeof(uint64_t));

    struct OutputMapKeyHash
    {
      std::size_t operator()(const OutputMapKey& value) const noexcept
      {
        return std::hash<uint64_t>{}(*reinterpret_cast<const uint64_t*>(&value));
      }
    };

    using OutputMap = std::unordered_map<OutputMapKey, std::shared_ptr<Output>, OutputMapKeyHash>;

  private:
    IdObject& interface();

  protected:
    OutputMap m_outputs;
    std::unordered_map<uint32_t, std::weak_ptr<OutputKeyboard>> m_outputKeyboards;

    OutputController(IdObject& interface);

    void addToWorld(OutputListColumn columns);
    void destroying();

  public:
    static constexpr std::vector<uint32_t>* noOutputChannels = nullptr;
    static constexpr uint32_t defaultOutputChannel = 0;

    ObjectProperty<OutputList> outputs;

    /**
     *
     */
    inline const OutputMap& outputMap() const { return m_outputs; }

    /**
     *
     */
    virtual const std::vector<uint32_t>* outputChannels() const { return noOutputChannels; }

    /**
     *
     */
    virtual const std::vector<std::string_view>* outputChannelNames() const { return nullptr; }

    /**
     *
     */
    bool isOutputChannel(uint32_t channel) const;

    /**
     *
     */
    virtual std::pair<uint32_t, uint32_t> outputAddressMinMax(uint32_t channel) const = 0;

    /**
     *
     */
    [[nodiscard]] virtual bool isOutputAddressAvailable(uint32_t channel, uint32_t address) const;

    /**
     * @brief Get the next unused output address
     *
     * @return An usused address or Output::invalidAddress if no unused address is available.
     */
    uint32_t getUnusedOutputAddress(uint32_t channel) const;

    /**
     *
     * @return \c true if changed, \c false otherwise.
     */
    [[nodiscard]] virtual bool changeOutputChannelAddress(Output& output, uint32_t newChannel, uint32_t newAddress);

    /**
     *
     * @return \c true if added, \c false otherwise.
     */
    [[nodiscard]] bool addOutput(Output& output);

    /**
     *
     * @return \c true if removed, \c false otherwise.
     */
    [[nodiscard]] bool removeOutput(Output& output);

    /**
     * @brief ...
     */
    [[nodiscard]] virtual bool setOutputValue(uint32_t channel, uint32_t address, bool value) = 0;

    /**
     * @brief Update the output value
     *
     * This function should be called by the hardware layer whenever the output value changes.
     *
     * @param[in] channel Output channel
     * @param[in] address Output address
     * @param[in] value New output value
     */
    void updateOutputValue(uint32_t channel, uint32_t address, TriState value);

    /**
     *
     *
     */
    std::shared_ptr<OutputKeyboard> outputKeyboard(uint32_t channel);
};

#endif
