/**
 * server/src/hardware/interface/Marklin6050Interface.cpp
 *
 * Copyright (C) 2025
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#endif

#include "Marklin6050Interface.hpp"
#include "../output/list/outputlist.hpp"
#include "../input/list/inputlist.hpp"
#include "../decoder/list/decoderlist.hpp"
#include "../decoder/list/decoderlisttablemodel.hpp"
#include "../decoder/decoderchangeflags.hpp"
#include "../decoder/decoder.hpp"
#include "../../utils/displayname.hpp"
#include "../../utils/makearray.hpp"
#include "../../world/world.hpp"
#include "../../core/attributes.hpp"
#include "../../core/objectproperty.tpp"
#include "../../core/eventloop.hpp"
#include "../../log/log.hpp"
#include "../../log/logmessageexception.hpp"

constexpr auto inputListColumns = InputListColumn::Address;
constexpr auto outputListColumns = OutputListColumn::Channel | OutputListColumn::Address;
constexpr auto decoderListColumns = DecoderListColumn::Id | DecoderListColumn::Name | DecoderListColumn::Address;

CREATE_IMPL(Marklin6050Interface)

Marklin6050Interface::Marklin6050Interface(World& world, std::string_view objId)
    : Interface(world, objId)
    , OutputController(static_cast<IdObject&>(*this))
    , InputController(static_cast<IdObject&>(*this))
    , DecoderController(*this, decoderListColumns)
    , serialPort(this, "serialPort", "", PropertyFlags::ReadWrite | PropertyFlags::Store)
    , baudrate(this, "baudrate", 2400, PropertyFlags::ReadWrite | PropertyFlags::Store)
    , settings{this, "settings", nullptr, PropertyFlags::ReadOnly | PropertyFlags::SubObject}
{
    name = "Märklin 6050/51";

    settings.setValueInternal(std::make_shared<Marklin6050::Settings>(*this, settings.name()));

    // --- Connection ---
    Attributes::addDisplayName(serialPort, DisplayName::Serial::device);
    Attributes::addEnabled(serialPort, !online);
    Attributes::addVisible(serialPort, true);
    m_interfaceItems.insertBefore(serialPort, notes);

    Attributes::addDisplayName(baudrate, DisplayName::Serial::baudrate);
    Attributes::addEnabled(baudrate, !online);
    Attributes::addVisible(baudrate, true);
    Attributes::addValues(baudrate, std::vector<unsigned int>{
        1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200
    });
    m_interfaceItems.insertBefore(baudrate, notes);

    // --- Settings ---
    m_interfaceItems.insertBefore(settings, notes);

    // --- Lists ---
    m_interfaceItems.insertBefore(inputs, notes);
    m_interfaceItems.insertBefore(outputs, notes);
    m_interfaceItems.insertBefore(decoders, notes);
}

// --- Lifecycle ---

void Marklin6050Interface::addToWorld()
{
    Interface::addToWorld();
    InputController::addToWorld(inputListColumns);
    OutputController::addToWorld(outputListColumns);
    DecoderController::addToWorld();
}

void Marklin6050Interface::loaded()
{
    Interface::loaded();
    updateEnabled();
}

void Marklin6050Interface::destroying()
{
    Interface::destroying();
    OutputController::destroying();
    InputController::destroying();
    DecoderController::destroying();
}

// --- State ---

void Marklin6050Interface::worldEvent(WorldState state, WorldEvent event)
{
    Interface::worldEvent(state, event);
    updateEnabled();

    if(!m_kernel)
        return;

    switch(event)
    {
        case WorldEvent::Stop:
            m_kernel->sendGlobalStop();
            break;

        case WorldEvent::Run:
            m_kernel->sendGlobalGo();
            break;

        default:
            break;
    }
}

void Marklin6050Interface::onlineChanged(bool /*value*/)
{
    updateEnabled();
}

void Marklin6050Interface::updateEnabled()
{
    Attributes::setEnabled(serialPort, !online);
    Attributes::setEnabled(baudrate, !online);
    settings->updateEnabled(online);
}

// --- Connection ---

bool Marklin6050Interface::setOnline(bool& value, bool simulation)
{
    setState(InterfaceState::Initializing);

    if(value)
    {
        m_simulation = simulation;

        if(!simulation)
        {
            const auto cfg = settings->config();

            try
            {
                m_kernel = std::make_unique<Marklin6050::Kernel>(id.value(), cfg);

                m_kernel->s88Callback = [this](uint32_t address, bool state)
                {
                    this->onS88Input(address, state);
                };

                if(cfg.extensions)
                {
                    m_kernel->extensionTurnoutCallback = [this](uint32_t address, bool green)
                    {
                        OutputPairValue val = green ? OutputPairValue::Second : OutputPairValue::First;
                        updateOutputValue(OutputChannel::Accessory, address, val);
                    };

                    m_kernel->extensionLocoCallback = [this](uint8_t address, uint8_t speed, bool f0, bool forward)
                    {
                        auto decoder = DecoderController::getDecoder(DecoderProtocol::Motorola, address);
                        if(!decoder)
                            return;

                        decoder->direction.setValueInternal(forward ? Direction::Forward : Direction::Reverse);
                        decoder->throttle.setValueInternal(Decoder::speedStepToThrottle<uint8_t>(speed, 14));
                        decoder->setFunctionValue(0, f0);
                    };

                    m_kernel->extensionFuncCallback = [this](uint8_t address, bool f1, bool f2, bool f3, bool f4)
                    {
                        auto decoder = DecoderController::getDecoder(DecoderProtocol::Motorola, address);
                        if(!decoder)
                            return;

                        decoder->setFunctionValue(1, f1);
                        decoder->setFunctionValue(2, f2);
                        decoder->setFunctionValue(3, f3);
                        decoder->setFunctionValue(4, f4);
                    };
                }

                m_kernel->start(serialPort, baudrate.value());
                m_kernel->startInputThread(cfg.s88amount, cfg.s88interval);

                if(cfg.extensions)
                    m_kernel->startExtensionThread();
            }
            catch(const LogMessageException& e)
            {
                Log::log(id.value(), e.message(), e.args());
                m_kernel.reset();
                value = false;
                setState(InterfaceState::Offline);
                return false;
            }
        }

        setState(InterfaceState::Online);
    }
    else
    {
        if(m_kernel)
        {
            m_kernel->stop();
            m_kernel.reset();
        }
        m_simulation = false;
        setState(InterfaceState::Offline);
    }

    updateEnabled();
    return true;
}
// --- Input ---

std::span<const InputChannel> Marklin6050Interface::inputChannels() const
{
    static const auto values = makeArray(InputChannel::S88);
    return values;
}

std::pair<uint32_t, uint32_t> Marklin6050Interface::inputAddressMinMax(InputChannel channel) const
{
    switch(channel)
    {
        case InputChannel::S88:
        {
            const uint32_t moduleCount = settings->s88amount.value();
            return {1, moduleCount * 16};
        }
        default:
            return {0, 0};
    }
}

void Marklin6050Interface::inputSimulateChange(InputChannel channel, uint32_t address, SimulateInputAction action)
{
    if(!m_simulation || channel != InputChannel::S88)
        return;

    switch(action)
    {
        case SimulateInputAction::SetFalse:
            onS88Input(address, false);
            break;

        case SimulateInputAction::SetTrue:
            onS88Input(address, true);
            break;

        case SimulateInputAction::Toggle:
            onS88Input(address, true);
            break;
    }
}

void Marklin6050Interface::onS88Input(uint32_t address, bool state)
{
    updateInputValue(InputChannel::S88, address, state ? TriState::True : TriState::False);
}

// --- Output ---

std::span<const OutputChannel> Marklin6050Interface::outputChannels() const
{
    static const auto values = makeArray(
        OutputChannel::Accessory,
        OutputChannel::Turnout,
        OutputChannel::Output
    );
    return values;
}

std::pair<uint32_t, uint32_t> Marklin6050Interface::outputAddressMinMax(OutputChannel channel) const
{
    switch(channel)
    {
        case OutputChannel::Accessory:
        case OutputChannel::Turnout:
        case OutputChannel::Output:
            return {1, 256};

        default:
            return OutputController::outputAddressMinMax(channel);
    }
}

bool Marklin6050Interface::setOutputValue(OutputChannel channel, uint32_t address, OutputValue value)
{
    if(!m_kernel || m_simulation)
        return false;

    switch(channel)
    {
        case OutputChannel::Accessory:
        case OutputChannel::Turnout:
        case OutputChannel::Output:
        {
            auto [min, max] = outputAddressMinMax(channel);
            if(address < min || address > max)
                return false;

            const bool result = m_kernel->setAccessory(address, value, settings->turnouttime.value());
            if(result)
                updateOutputValue(channel, address, value);

            return result;
        }
        default:
            return false;
    }
}

// --- Decoder ---

std::span<const DecoderProtocol> Marklin6050Interface::decoderProtocols() const
{
    const uint16_t ver = settings->centralUnitVersion;
    const bool isAnalog = settings->analog;

    const bool isDcc =
        ((ver == 6027 || ver == 6029 || ver == 6030 || ver == 6032) && !isAnalog);

    const bool isNone =
        ((ver == 6027 || ver == 6029) && isAnalog);

    if(isDcc)
    {
        static constexpr std::array<DecoderProtocol, 1> protocols{DecoderProtocol::DCCShort};
        return protocols;
    }
    if(isNone)
    {
        static constexpr std::array<DecoderProtocol, 1> protocols{DecoderProtocol::None};
        return protocols;
    }

    // 6020, 6021, 6022, 6023, 6223
    static constexpr std::array<DecoderProtocol, 1> protocols{DecoderProtocol::Motorola};
    return protocols;
}

std::pair<uint16_t, uint16_t> Marklin6050Interface::decoderAddressMinMax(DecoderProtocol /*protocol*/) const
{
    const uint16_t ver = settings->centralUnitVersion;
    const bool isAnalog = settings->analog;
    const bool ext = settings->extensions;

    const bool isDcc =
        ((ver == 6027 || ver == 6029 || ver == 6030 || ver == 6032) && !isAnalog);

    const bool isNone =
        ((ver == 6027 || ver == 6029) && isAnalog);

    const bool isLimited =
        (ver == 6022 || ver == 6023 || ver == 6223);

    if(isDcc)
        return ext ? std::pair{uint16_t(1), uint16_t(127)}
                   : std::pair{uint16_t(1), uint16_t(80)};

    if(isNone)
        return {1, 1};

    if(ver == 6021)
        return ext ? std::pair{uint16_t(1), uint16_t(255)}
                   : std::pair{uint16_t(1), uint16_t(80)};

    if(ver == 6020)
        return {1, 80};

    if(isLimited)
        return {10, 40};

    return {0, 0};
}

std::span<const uint8_t> Marklin6050Interface::decoderSpeedSteps(DecoderProtocol /*protocol*/) const
{
    static constexpr uint8_t steps[] = {14};
    return steps;
}

void Marklin6050Interface::decoderChanged(
    const Decoder& decoder,
    DecoderChangeFlags changes,
    uint32_t functionNumber)
{
    if(!m_kernel || m_simulation)
        return;

    const uint8_t address = static_cast<uint8_t>(decoder.address);
    const bool f0 = decoder.getFunctionValue(0);
    const uint8_t speed = decoder.emergencyStop
        ? 0
        : Decoder::throttleToSpeedStep<uint8_t>(decoder.throttle, 14);

    const uint16_t ver = settings->centralUnitVersion;
    const bool isAnalog = settings->analog;

    // function support tiers
    const bool noFunctions =
        ((ver == 6027 || ver == 6029) && isAnalog);

    const bool f0only =
        (ver == 6020 || ver == 6022 || ver == 6023 || ver == 6223);

    const bool effectiveF0 = noFunctions ? false : f0;

    // emergency stop: double direction toggle
    if(has(changes, DecoderChangeFlags::EmergencyStop) && decoder.emergencyStop)
    {
        m_kernel->setLocoEmergencyStop(address, effectiveF0);
        return;
    }

    // direction toggle
    if(has(changes, DecoderChangeFlags::Direction))
    {
        m_kernel->setLocoDirection(address, effectiveF0);
        return;
    }

    // speed update (including resume from e-stop)
    if(has(changes, DecoderChangeFlags::EmergencyStop | DecoderChangeFlags::Throttle))
    {
        m_kernel->setLocoSpeed(address, speed, effectiveF0);
        return;
    }

    // function changes
    if(has(changes, DecoderChangeFlags::FunctionValue))
    {
        if(functionNumber == 0 && !noFunctions)
        {
            m_kernel->setLocoFunction(address, speed, f0);
        }
        else if(functionNumber <= 4 && !noFunctions && !f0only)
        {
            m_kernel->setLocoFunctions1to4(
                address,
                decoder.getFunctionValue(1),
                decoder.getFunctionValue(2),
                decoder.getFunctionValue(3),
                decoder.getFunctionValue(4));
        }
    }
}
