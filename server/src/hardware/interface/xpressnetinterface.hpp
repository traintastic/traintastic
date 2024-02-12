/**
 * server/src/hardware/interface/xpressnetinterface.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_XPRESSNETINTERFACE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_XPRESSNETINTERFACE_HPP

#include "interface.hpp"
#include "../decoder/decodercontroller.hpp"
#include "../input/inputcontroller.hpp"
#include "../output/outputcontroller.hpp"
#include "../../core/serialdeviceproperty.hpp"
#include "../../core/objectproperty.hpp"
#include "../../enum/xpressnetinterfacetype.hpp"
#include "../../enum/xpressnetserialinterfacetype.hpp"
#include "../../enum/serialflowcontrol.hpp"

namespace XpressNet {
class Kernel;
class Settings;
}

/**
 * @brief XpressNet hardware interface
 */
class XpressNetInterface final
  : public Interface
  , public DecoderController
  , public InputController
  , public OutputController
{
  CLASS_ID("interface.xpressnet")
  DEFAULT_ID("xpressnet")
  CREATE_DEF(XpressNetInterface)

  private:
    std::unique_ptr<XpressNet::Kernel> m_kernel;
    boost::signals2::connection m_xpressnetPropertyChanged;

    void addToWorld() final;
    void loaded() final;
    void destroying() final;
    void worldEvent(WorldState state, WorldEvent event) final;

    void updateVisible();

  protected:
    bool setOnline(bool& value, bool simulation) final;

  public:
    Property<XpressNetInterfaceType> type;
    Property<XpressNetSerialInterfaceType> serialInterfaceType;
    SerialDeviceProperty device;
    Property<uint32_t> baudrate;
    Property<SerialFlowControl> flowControl;
    Property<std::string> hostname;
    Property<uint16_t> port;
    Property<uint8_t> s88StartAddress;
    Property<uint8_t> s88ModuleCount;
    ObjectProperty<XpressNet::Settings> xpressnet;

    XpressNetInterface(World& world, std::string_view _id);

    // DecoderController:
    tcb::span<const DecoderProtocol> decoderProtocols() const final;
    std::pair<uint16_t, uint16_t> decoderAddressMinMax(DecoderProtocol protocol) const final;
    tcb::span<const uint8_t> decoderSpeedSteps(DecoderProtocol protocol) const final;
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber) final;

    // InputController:
    std::pair<uint32_t, uint32_t> inputAddressMinMax(uint32_t /*channel*/) const final;
    void inputSimulateChange(uint32_t channel, uint32_t address, SimulateInputAction action) final;

    // OutputController:
    tcb::span<const OutputChannel> outputChannels() const final;
    std::pair<uint32_t, uint32_t> outputAddressMinMax(OutputChannel /*channel*/) const final;
    [[nodiscard]] bool setOutputValue(OutputChannel channel, uint32_t address, OutputValue value) final;
};

#endif
