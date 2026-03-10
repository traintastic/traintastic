/**
 * server/src/hardware/interface/Marklin6050Interface.hpp
 *
 * Copyright (C) 2025
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_MARKLIN6050INTERFACE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_MARKLIN6050INTERFACE_HPP

#include "interface.hpp"
#include "../../core/objectproperty.hpp"
#include "../output/outputcontroller.hpp"
#include "../input/inputcontroller.hpp"
#include "../decoder/decodercontroller.hpp"
#include "../../core/serialdeviceproperty.hpp"
#include "../../hardware/protocol/Marklin6050Interface/kernel.hpp"
#include "../../hardware/protocol/Marklin6050Interface/settings.hpp"

class Marklin6050Interface
    : public Interface
    , public OutputController
    , public InputController
    , public DecoderController
{
    CLASS_ID("interface.marklin6050")
    DEFAULT_ID("marklin6050")
    CREATE_DEF(Marklin6050Interface)

private:
    std::unique_ptr<Marklin6050::Kernel> m_kernel;
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
    SerialDeviceProperty serialPort;
    Property<uint32_t> baudrate;
    ObjectProperty<Marklin6050::Settings> settings;

    Marklin6050Interface(World& world, std::string_view id);

    // DecoderController:
    std::span<const DecoderProtocol> decoderProtocols() const final;
    std::pair<uint16_t, uint16_t> decoderAddressMinMax(DecoderProtocol protocol) const final;
    std::span<const uint8_t> decoderSpeedSteps(DecoderProtocol protocol) const final;
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber) final;

    // InputController:
    std::span<const InputChannel> inputChannels() const final;
    std::pair<uint32_t, uint32_t> inputAddressMinMax(InputChannel channel) const final;
    void inputSimulateChange(InputChannel channel, uint32_t address, SimulateInputAction action) final;

    // OutputController:
    std::span<const OutputChannel> outputChannels() const final;
    std::pair<uint32_t, uint32_t> outputAddressMinMax(OutputChannel channel) const final;
    [[nodiscard]] bool setOutputValue(OutputChannel channel, uint32_t address, OutputValue value) final;
};

#endif
