/**
 * server/src/hardware/interface/loconetinterface.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_LOCONETINTERFACE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_LOCONETINTERFACE_HPP

#include "interface.hpp"
#include "../decoder/decodercontroller.hpp"
#include "../input/inputcontroller.hpp"
#include "../output/outputcontroller.hpp"
#include "../identification/identificationcontroller.hpp"
#include "../programming/lncv/lncvprogrammingcontroller.hpp"
#include "../../core/serialdeviceproperty.hpp"
#include "../../core/objectproperty.hpp"
#include "../../enum/loconetinterfacetype.hpp"
#include "../../enum/serialflowcontrol.hpp"
#include <tcb/span.hpp>

namespace LocoNet {
class Kernel;
class Settings;
}

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
  CREATE_DEF(LocoNetInterface)

  private:
    std::unique_ptr<LocoNet::Kernel> m_kernel;
    boost::signals2::connection m_loconetPropertyChanged;

    void addToWorld() final;
    void loaded() final;
    void destroying() final;
    void worldEvent(WorldState state, WorldEvent event) final;

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

    //! \brief Send LocoNet packet
    //! \param[in] packet LocoNet packet bytes, exluding checksum.
    //! \return \c true if send, \c false otherwise.
    bool send(tcb::span<uint8_t> packet);

    //! \brief Send immediate DCC packet
    //! \param[in] dccPacket DCC packet byte, exluding checksum. Length is limited to 5.
    //! \param[in] repeat DCC packet repeat count 0..7
    //! \return \c true if send to command station, \c false otherwise.
    bool immPacket(tcb::span<uint8_t> dccPacket, uint8_t repeat);

    // DecoderController:
    tcb::span<const DecoderProtocol> decoderProtocols() const final;
    std::pair<uint16_t, uint16_t> decoderAddressMinMax(DecoderProtocol protocol) const final;
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber) final;

    // InputController:
    std::pair<uint32_t, uint32_t> inputAddressMinMax(uint32_t /*channel*/) const final;
    void inputSimulateChange(uint32_t channel, uint32_t address, SimulateInputAction action) final;

    // OutputController:
    tcb::span<const OutputChannel> outputChannels() const final;
    std::pair<uint32_t, uint32_t> outputAddressMinMax(OutputChannel /*channel*/) const final;
    [[nodiscard]] bool setOutputValue(OutputChannel channel, uint32_t address, OutputValue value) final;

    // IdentificationController:
    std::pair<uint32_t, uint32_t> identificationAddressMinMax(uint32_t /*channel*/) const final;
    void identificationEvent(uint32_t channel, uint32_t address, IdentificationEventType eventType, uint16_t identifier, Direction direction, uint8_t category) final;

    // LNCVProgrammingController:
    bool startLNCVProgramming(uint16_t moduleId, uint16_t moduleAddress) final;
    bool readLNCV(uint16_t lncv) final;
    bool writeLNCV(uint16_t lncv, uint16_t value) final;
    bool stopLNCVProgramming() final;
};

#endif
