/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Kamil Kasprzak
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



#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_MARKLIN6023INTERFACE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_MARKLIN6023INTERFACE_HPP

#include "interface.hpp"
#include "../../core/objectproperty.hpp"
#include "../../core/serialdeviceproperty.hpp"
#include "../output/outputcontroller.hpp"
#include "../input/inputcontroller.hpp"
#include "../decoder/decodercontroller.hpp"
#include "../protocol/Marklin6023Interface/kernel.hpp"
#include "../protocol/Marklin6023Interface/settings.hpp"

class Marklin6023Interface final
    : public Interface
    , public OutputController
    , public InputController
    , public DecoderController
{
    CLASS_ID("interface.marklin6023")
    DEFAULT_ID("marklin6023")
    CREATE_DEF(Marklin6023Interface)

private:
    std::unique_ptr<Marklin6023::Kernel> m_kernel;
    bool m_simulation{false};

    void updateEnabled();
    void onS88Input(uint32_t address, bool state);

protected:
    void addToWorld() final;
    void loaded() final;
    void destroying() final;
    void worldEvent(WorldState state, WorldEvent event) final;
    void onlineChanged(bool value);
    bool setOnline(bool& value, bool simulation) final;

public:
    SerialDeviceProperty          serialPort;
    Property<uint32_t>            baudrate;
    ObjectProperty<Marklin6023::Settings> settings;

    Marklin6023Interface(World& world, std::string_view id);

    // DecoderController
    std::span<const DecoderProtocol> decoderProtocols() const final;
    std::pair<uint16_t, uint16_t>    decoderAddressMinMax(DecoderProtocol protocol) const final;
    std::span<const uint8_t>         decoderSpeedSteps(DecoderProtocol protocol) const final;
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes,
                        uint32_t functionNumber) final;

    // InputController
    std::span<const InputChannel>      inputChannels() const final;
    std::pair<uint32_t, uint32_t>      inputAddressMinMax(InputChannel channel) const final;
    void inputSimulateChange(InputChannel channel, uint32_t address,
                             SimulateInputAction action) final;

    // OutputController
    std::span<const OutputChannel>     outputChannels() const final;
    std::pair<uint32_t, uint32_t>      outputAddressMinMax(OutputChannel channel) const final;
    [[nodiscard]] bool setOutputValue(OutputChannel channel, uint32_t address,
                                      OutputValue value) final;
};

#endif
