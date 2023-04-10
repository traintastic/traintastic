/**
 * server/src/hardware/interface/dccplusplusinterface.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_DCCPLUSPLUSINTERFACE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_DCCPLUSPLUSINTERFACE_HPP

#include "interface.hpp"
#include "../protocol/dccplusplus/kernel.hpp"
#include "../protocol/dccplusplus/settings.hpp"
#include "../decoder/decodercontroller.hpp"
#include "../input/inputcontroller.hpp"
#include "../output/outputcontroller.hpp"
#include "../../core/serialdeviceproperty.hpp"
#include "../../core/objectproperty.hpp"
#include "../../enum/serialflowcontrol.hpp"

/**
 * @brief DCC++(EX) hardware interface
 */
class DCCPlusPlusInterface final
  : public Interface
  , public DecoderController
  , public InputController
  , public OutputController
{
  CLASS_ID("interface.dccplusplus")
  CREATE(DCCPlusPlusInterface)
  DEFAULT_ID("dccplusplus")

  private:
    std::unique_ptr<DCCPlusPlus::Kernel> m_kernel;
    boost::signals2::connection m_dccplusplusPropertyChanged;

    void addToWorld() final;
    void loaded() final;
    void destroying() final;
    void worldEvent(WorldState state, WorldEvent event) final;

    void check() const;
    void checkDecoder(const Decoder& decoder) const;

    void idChanged(const std::string& newId) final;

  protected:
    bool setOnline(bool& value, bool simulation) final;

  public:
    SerialDeviceProperty device;
    Property<uint32_t> baudrate;
    ObjectProperty<DCCPlusPlus::Settings> dccplusplus;

    DCCPlusPlusInterface(World& world, std::string_view _id);

    // DecoderController:
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber) final;

    // InputController:
    std::pair<uint32_t, uint32_t> inputAddressMinMax(uint32_t /*channel*/) const final { return {DCCPlusPlus::Kernel::idMin, DCCPlusPlus::Kernel::idMax}; }
    void inputSimulateChange(uint32_t channel, uint32_t address, SimulateInputAction action) final;

    // OutputController:
    const std::vector<uint32_t>* outputChannels() const final { return &DCCPlusPlus::Kernel::outputChannels; }
    const std::vector<std::string_view>* outputChannelNames() const final { return &DCCPlusPlus::Kernel::outputChannelNames; }
    std::pair<uint32_t, uint32_t> outputAddressMinMax(uint32_t channel) const final;
    [[nodiscard]] bool setOutputValue(uint32_t channel, uint32_t address, bool value) final;
};

#endif
