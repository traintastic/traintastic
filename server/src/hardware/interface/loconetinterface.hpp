/**
 * server/src/hardware/interface/loconetinterface.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_LOCONETINTERFACE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_LOCONETINTERFACE_HPP

#include "interface.hpp"
#include "../protocol/loconet/kernel.hpp"
#include "../protocol/loconet/settings.hpp"
#include "../decoder/decodercontroller.hpp"
#include "../input/inputcontroller.hpp"
#include "../output/outputcontroller.hpp"
#include "../identification/identificationcontroller.hpp"
#include "../programming/lncv/lncvprogrammingcontroller.hpp"
#include "../../core/serialdeviceproperty.hpp"
#include "../../core/objectproperty.hpp"
#include "../../enum/loconetinterfacetype.hpp"
#include "../../enum/serialflowcontrol.hpp"

/**
 * @brief LocoNet hardware interface
 */
class LocoNetInterface final
  : public Interface
  , public DecoderController
  , public InputController
  , public OutputController
  , public IdentificationController
  , public LNCVProgrammingController
{
  CLASS_ID("interface.loconet")
  DEFAULT_ID("loconet")
  CREATE(LocoNetInterface)

  private:
    std::unique_ptr<LocoNet::Kernel> m_kernel;
    boost::signals2::connection m_loconetPropertyChanged;

    void addToWorld() final;
    void loaded() final;
    void destroying() final;
    void worldEvent(WorldState state, WorldEvent event) final;

    void idChanged(const std::string& newId) final;

    void typeChanged();

  protected:
    bool setOnline(bool& value, bool simulation) final;

  public:
    Property<LocoNetInterfaceType> type;
    SerialDeviceProperty device;
    Property<uint32_t> baudrate;
    Property<SerialFlowControl> flowControl;
    Property<std::string> hostname;
    Property<uint16_t> port;
    ObjectProperty<LocoNet::Settings> loconet;

    LocoNetInterface(World& world, std::string_view _id);

    // DecoderController:
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber) final;

    // InputController:
    std::pair<uint32_t, uint32_t> inputAddressMinMax(uint32_t /*channel*/) const final { return {LocoNet::Kernel::inputAddressMin, LocoNet::Kernel::inputAddressMax}; }
    void inputSimulateChange(uint32_t channel, uint32_t address) final;

    // OutputController:
    std::pair<uint32_t, uint32_t> outputAddressMinMax(uint32_t /*channel*/) const final { return {LocoNet::Kernel::outputAddressMin, LocoNet::Kernel::outputAddressMax}; }
    [[nodiscard]] bool setOutputValue(uint32_t channel, uint32_t address, bool value) final;

    // IdentificationController:
    std::pair<uint32_t, uint32_t> identificationAddressMinMax(uint32_t /*channel*/) const final { return {LocoNet::Kernel::identificationAddressMin, LocoNet::Kernel::identificationAddressMax}; }
    void identificationEvent(uint32_t channel, uint32_t address, IdentificationEventType eventType, uint16_t identifier, Direction direction, uint8_t category) final;

    // LNCVProgrammingController:
    bool startLNCVProgramming(uint16_t moduleId, uint16_t moduleAddress) final;
    bool readLNCV(uint16_t lncv) final;
    bool writeLNCV(uint16_t lncv, uint16_t value) final;
    bool stopLNCVProgramming() final;
};

#endif
