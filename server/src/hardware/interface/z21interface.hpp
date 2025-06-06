/**
 * server/src/hardware/interface/z21interface.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_Z21INTERFACE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_Z21INTERFACE_HPP

#include "interface.hpp"
#include "../decoder/decodercontroller.hpp"
#include "../input/inputcontroller.hpp"
#include "../output/outputcontroller.hpp"
#include "../../core/objectproperty.hpp"

namespace Z21 {
class ClientKernel;
class ClientSettings;
}

/**
 * @brief Z21 hardware interface
 */
class Z21Interface final
  : public Interface
  , public DecoderController
  , public InputController
  , public OutputController
{
  CLASS_ID("interface.z21")
  DEFAULT_ID("z21")
  CREATE_DEF(Z21Interface)

  private:
    std::unique_ptr<Z21::ClientKernel> m_kernel;
    boost::signals2::connection m_z21PropertyChanged;

    void addToWorld() final;
    void destroying() final;
    void worldEvent(WorldState state, WorldEvent event) final;

    void updateVisible();

  protected:
    bool setOnline(bool& value, bool simulation) final;

  public:
    Property<std::string> hostname;
    Property<uint16_t> port;
    ObjectProperty<Z21::ClientSettings> z21;
    Property<std::string> hardwareType;
    Property<std::string> serialNumber;
    Property<std::string> firmwareVersion;

    Z21Interface(World& world, std::string_view _id);

    // DecoderController:
    std::span<const DecoderProtocol> decoderProtocols() const final;
    std::pair<uint16_t, uint16_t> decoderAddressMinMax(DecoderProtocol protocol) const final;
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber) final;

    // InputController:
    const std::vector<uint32_t>* inputChannels() const final;
    const std::vector<std::string_view>* inputChannelNames() const final;
    std::pair<uint32_t, uint32_t> inputAddressMinMax(uint32_t channel) const final;
    void inputSimulateChange(uint32_t channel, uint32_t address, SimulateInputAction action) final;

    // OutputController:
    std::span<const OutputChannel> outputChannels() const final;
    std::pair<uint32_t, uint32_t> outputAddressMinMax(OutputChannel /*channel*/) const final;
    [[nodiscard]] bool setOutputValue(OutputChannel channel, uint32_t address, OutputValue value) final;
};

#endif
