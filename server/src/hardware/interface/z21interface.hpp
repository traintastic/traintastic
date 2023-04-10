/**
 * server/src/hardware/interface/z21interface.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2023 Reinder Feenstra
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
#include "../protocol/z21/clientkernel.hpp"
#include "../protocol/z21/clientsettings.hpp"
#include "../decoder/decodercontroller.hpp"
#include "../input/inputcontroller.hpp"
#include "../output/outputcontroller.hpp"
#include "../../core/objectproperty.hpp"

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
  CREATE(Z21Interface)

  private:
    std::unique_ptr<Z21::ClientKernel> m_kernel;
    boost::signals2::connection m_z21PropertyChanged;

    void addToWorld() final;
    void destroying() final;
    void worldEvent(WorldState state, WorldEvent event) final;

    void idChanged(const std::string& newId) final;

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
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber) final;

    // InputController:
    const std::vector<uint32_t>* inputChannels() const final { return &Z21::ClientKernel::inputChannels; }
    const std::vector<std::string_view>* inputChannelNames() const final { return &Z21::ClientKernel::inputChannelNames; }
    std::pair<uint32_t, uint32_t> inputAddressMinMax(uint32_t channel) const final;
    void inputSimulateChange(uint32_t channel, uint32_t address, SimulateInputAction action) final;

    // OutputController:
    std::pair<uint32_t, uint32_t> outputAddressMinMax(uint32_t /*channel*/) const final { return {Z21::ClientKernel::outputAddressMin, Z21::ClientKernel::outputAddressMax}; }
    [[nodiscard]] bool setOutputValue(uint32_t channel, uint32_t address, bool value) final;
};

#endif
