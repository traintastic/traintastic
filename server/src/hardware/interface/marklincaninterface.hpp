/**
 * server/src/hardware/interface/marklincaninterface.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023-2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_MARKLINCANINTERFACE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_MARKLINCANINTERFACE_HPP

#include "interface.hpp"
#include "marklincan/marklincannodelist.hpp"
#include "marklincan/marklincanlocomotivelist.hpp"
#include <traintastic/enum/marklincaninterfacetype.hpp>
#include "../decoder/decodercontroller.hpp"
#include "../input/inputcontroller.hpp"
#include "../output/outputcontroller.hpp"
#include "../protocol/marklincan/kernel.hpp"
#include "../protocol/marklincan/settings.hpp"
#include "../../core/serialdeviceproperty.hpp"
#include "../../enum/serialflowcontrol.hpp"

/**
 * @brief Märklin CAN hardware interface
 */
class MarklinCANInterface final
  : public Interface
  , public DecoderController
  , public InputController
  , public OutputController
{
  CLASS_ID("interface.marklin_can")
  CREATE(MarklinCANInterface)
  DEFAULT_ID("marklin_can")

  friend class MarklinCANLocomotiveList;

  private:
    std::unique_ptr<MarklinCAN::Kernel> m_kernel;
    boost::signals2::connection m_marklinCANPropertyChanged;

    void addToWorld() final;
    void loaded() final;
    void destroying() final;
    void worldEvent(WorldState state, WorldEvent event) final;

    void typeChanged();

  protected:
    bool setOnline(bool& value, bool simulation) final;

  public:
    Property<MarklinCANInterfaceType> type;
    Property<std::string> hostname;
    Property<std::string> interface;
    SerialDeviceProperty device;
    Property<uint32_t> baudrate;
    Property<SerialFlowControl> flowControl;
    ObjectProperty<MarklinCAN::Settings> marklinCAN;
    ObjectProperty<MarklinCANNodeList> marklinCANNodeList;
    ObjectProperty<MarklinCANLocomotiveList> marklinCANLocomotiveList;

    MarklinCANInterface(World& world, std::string_view _id);

    // DecoderController:
    tcb::span<const DecoderProtocol> decoderProtocols() const final;
    tcb::span<const uint8_t> decoderSpeedSteps(DecoderProtocol protocol) const final;
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber) final;

    // InputController:
    std::pair<uint32_t, uint32_t> inputAddressMinMax(uint32_t /*channel*/) const final;

    // OutputController:
    tcb::span<const OutputChannel> outputChannels() const final;
    [[nodiscard]] bool setOutputValue(OutputChannel channel, uint32_t address, OutputValue value) final;
};

#endif
