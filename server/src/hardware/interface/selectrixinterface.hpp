/**
 * server/src/hardware/interface/selectrixinterface.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_SELECTRIXINTERFACE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_SELECTRIXINTERFACE_HPP

#include "interface.hpp"
#include "../decoder/decodercontroller.hpp"
#include "../input/inputcontroller.hpp"
#include "../output/outputcontroller.hpp"
#include "../../core/serialdeviceproperty.hpp"
#include "../../core/objectproperty.hpp"
#include "../../enum/serialflowcontrol.hpp"
#include <tcb/span.hpp>

namespace Selectrix {
  class Kernel;
  class Settings;
  enum class Bus : uint8_t;
  enum class AddressType : uint8_t;
}

/**
 * \brief Selectrix hardware interface
 */
class SelectrixInterface final
  : public Interface
  , public DecoderController
  , public InputController
{
  CLASS_ID("interface.selectrix")
  DEFAULT_ID("selectrix")
  CREATE_DEF(SelectrixInterface)

  private:
    static constexpr uint32_t inputAddressMin = 1;
    static constexpr uint32_t inputAddressMax = 112 * 8;  // Address: 0 .. 111, 8 inputs per address
    static constexpr uint32_t inputAddressMaxSX0 = 104 * 8; // Address: 0 .. 103, 8 inputs per address

    struct BusAddress
    {
      Selectrix::Bus bus;
      uint8_t address;
    };
    friend constexpr bool operator <(const SelectrixInterface::BusAddress& lhs, const SelectrixInterface::BusAddress& rhs);

    struct BusAddressUsage
    {
      Selectrix::AddressType type;
      uint8_t mask;
    };

    struct InputChannel
    {
      // zero is reserved for the defaultChannel
      static constexpr uint32_t sx0 = 1;
      static constexpr uint32_t sx1 = 2;
      static constexpr uint32_t sx2 = 3;
    };

    inline static const std::vector<uint32_t> channels = {
      InputChannel::sx0,
      InputChannel::sx1,
      InputChannel::sx2,
    };

    inline static const std::vector<std::string_view> channelNames = {
      "SX0",
      "SX1",
      "SX2",
    };

    std::unique_ptr<Selectrix::Kernel> m_kernel;
    boost::signals2::connection m_selectrixPropertyChanged;
    std::map<BusAddress, BusAddressUsage> m_usedBusAddresses;

  protected:
    void addToWorld() final;
    void loaded() final;
    void destroying() final;
    void worldEvent(WorldState state, WorldEvent event) final;

    bool setOnline(bool& value, bool simulation) final;

    // InputController:
    void inputAdded(Input& input) final;
    void inputRemoved(Input& input) final;

  public:
    SerialDeviceProperty device;
    Property<uint32_t> baudrate;
    Property<SerialFlowControl> flowControl;
    ObjectProperty<Selectrix::Settings> selectrix;

    SelectrixInterface(World& world, std::string_view _id);

    // DecoderController:
    tcb::span<const DecoderProtocol> decoderProtocols() const final;
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber) final;

    // InputController:
    const std::vector<uint32_t>* inputChannels() const final { return &channels; }
    const std::vector<std::string_view>* inputChannelNames() const final { return &channelNames; }
    std::pair<uint32_t, uint32_t> inputAddressMinMax(uint32_t channel) const final;
    [[nodiscard]] bool isInputAddressAvailable(uint32_t channel, uint32_t address) const final;
    void inputSimulateChange(uint32_t channel, uint32_t address, SimulateInputAction action) final;
};

#endif
