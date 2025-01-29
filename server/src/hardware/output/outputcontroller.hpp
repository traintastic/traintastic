/**
 * server/src/hardware/output/outputcontroller.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022,2024-2025 Reinder Feenstra
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
#include <boost/signals2/signal.hpp>
#include <span>
#include <traintastic/enum/outputchannel.hpp>
#include <traintastic/enum/outputtype.hpp>
#include "outputvalue.hpp"
#include "../../core/objectproperty.hpp"

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
    static constexpr std::pair<uint32_t, uint32_t> noAddressMinMax{std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::min()};

    struct OutputMapKey
    {
      OutputChannel channel;
      uint32_t id;

      inline bool operator ==(const OutputMapKey other) const noexcept
      {
        return channel == other.channel && id == other.id;
      }
    };
    static_assert(sizeof(OutputMapKey) == sizeof(uint64_t));

    struct OutputMapKeyHash
    {
      std::size_t operator()(const OutputMapKey& value) const noexcept
      {
        return (static_cast<uint64_t>(value.channel) << (8 * sizeof(value.id))) | value.id;
      }
    };

    using OutputMap = std::unordered_map<OutputMapKey, std::shared_ptr<Output>, OutputMapKeyHash>;

  private:
    std::shared_ptr<OutputController> shared_ptr();
    IdObject& interface();

  protected:
    OutputMap m_outputs;
    std::unordered_map<OutputChannel, std::weak_ptr<OutputKeyboard>> m_outputKeyboards;

    OutputController(IdObject& interface);

    void addToWorld(OutputListColumn columns);
    void destroying();

  public:
    boost::signals2::signal<void()> outputECoSObjectsChanged;

    ObjectProperty<OutputList> outputs;

    /**
     *
     */
    inline const OutputMap& outputMap() const { return m_outputs; }

    /**
     *
     */
    virtual std::span<const OutputChannel> outputChannels() const = 0;

    /**
     *
     */
    bool isOutputChannel(OutputChannel channel) const;

    /**
     *
     */
    virtual bool isOutputId(OutputChannel channel, uint32_t id) const;

    /**
     *
     */
    virtual OutputType outputType(OutputChannel channel) const;

    /**
     *
     */
    virtual std::pair<uint32_t, uint32_t> outputAddressMinMax(OutputChannel channel) const;

    /**
     *
     */
    virtual std::pair<std::span<const uint16_t>, std::span<const std::string>> getOutputECoSObjects(OutputChannel channel) const;

    /**
     *
     */
    [[nodiscard]] virtual bool isOutputAvailable(OutputChannel channel, uint32_t id) const;

    /**
     * \brief Get the next unused output address
     *
     * \param[in] channel The output channel.
     * \return An usused address or Output::invalidAddress if no unused address is available.
     */
    uint32_t getUnusedOutputAddress(OutputChannel channel) const;

    /**
     * \brief Get an output.
     *
     * For each channel/id combination an output object is create once,
     * if an output is requested multiple time they all share the same instance.
     * Once the object the uses the output is no longer using it,
     * it must be released using \ref releaseOutput .
     * The output object will be destroyed when the are zero users.
     *
     * \param[in] channel The output channel.
     * \param[in] id The output id.
     * \param[in] usedBy The object the will use the output.
     * \return An output object if the channel/id combination is valid, \c nullptr otherwise.
     */
    std::shared_ptr<Output> getOutput(OutputChannel channel, uint32_t id, Object& usedBy);

    /**
     * \brief Release an output.
     *
     * \param[in] output The output to release.
     * \param[in] usedBy The object no longer using the output.
     * \see getOutput
     */
    void releaseOutput(Output& output, Object& usedBy);

    /**
     * @brief ...
     */
    [[nodiscard]] virtual bool setOutputValue(OutputChannel /*channel*/, uint32_t /*id*/, OutputValue /*value*/) = 0;

    /**
     * @brief Update the output value
     *
     * This function should be called by the hardware layer whenever the output value changes.
     *
     * @param[in] channel Output channel
     * @param[in] id Output id
     * @param[in] value New output value
     */
    void updateOutputValue(OutputChannel channel, uint32_t id, OutputValue value);

    /**
     * \brief Check is there is an output keyboard available for the channel.
     *
     * \param[in] channel The output channel
     * \return \c true if available, \c false if not.
     */
    bool hasOutputKeyboard(OutputChannel channel) const;

    /**
     * \brief Get an output keyboard for the channel if available.
     *
     * \param[in] channel The output channel
     * \return An output keyboard if available, \c nullptr otherwise.
     */
    std::shared_ptr<OutputKeyboard> outputKeyboard(OutputChannel channel);
};

#endif
