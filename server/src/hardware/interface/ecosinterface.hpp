/**
 * server/src/hardware/interface/ecosinterface.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_ECOSINTERFACE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_ECOSINTERFACE_HPP

#include "interface.hpp"
#include <filesystem>
#include "../protocol/ecos/simulation.hpp"
#include "../decoder/decodercontroller.hpp"
#include "../input/inputcontroller.hpp"
#include "../output/outputcontroller.hpp"
#include "../../core/objectproperty.hpp"

namespace ECoS {
class Kernel;
class Settings;
}

/**
 * @brief ECoS hardware interface
 */
class ECoSInterface final
  : public Interface
  , public DecoderController
  , public InputController
  , public OutputController
{
  CLASS_ID("interface.ecos")
  DEFAULT_ID("ecos")
  CREATE_DEF(ECoSInterface)

  private:
    std::unique_ptr<ECoS::Kernel> m_kernel;
    boost::signals2::connection m_ecosPropertyChanged;
    ECoS::Simulation m_simulation;
    std::vector<uint16_t> m_outputECoSObjectIds;
    std::vector<std::string> m_outputECoSObjectNames;

    void addToWorld() final;
    void destroying() final;
    void load(WorldLoader& loader, const nlohmann::json& data) final;
    void save(WorldSaver& saver, nlohmann::json& data, nlohmann::json& state) const final;
    void worldEvent(WorldState state, WorldEvent event) final;

    void typeChanged();

    std::filesystem::path simulationDataFilename() const;

  protected:
    bool setOnline(bool& value, bool simulation) final;

  public:
    Property<std::string> hostname;
    ObjectProperty<ECoS::Settings> ecos;

    ECoSInterface(World& world, std::string_view _id);

    // DecoderController:
    std::span<const DecoderProtocol> decoderProtocols() const final;
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber) final;

    // InputController:
    const std::vector<uint32_t>* inputChannels() const final;
    const std::vector<std::string_view>* inputChannelNames() const final;
    std::pair<uint32_t, uint32_t> inputAddressMinMax(uint32_t channel) const final;
    void inputSimulateChange(uint32_t channel, uint32_t address, SimulateInputAction action) final;

    // OutputController:
    std::span<const OutputChannel> outputChannels() const final;
    std::pair<std::span<const uint16_t>, std::span<const std::string>> getOutputECoSObjects(OutputChannel channel) const final;
    bool isOutputId(OutputChannel channel, uint32_t id) const final;
    [[nodiscard]] bool setOutputValue(OutputChannel channel, uint32_t outputId, OutputValue value) final;
};

#endif
