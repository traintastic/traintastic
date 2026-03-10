/**
 * server/src/hardware/protocol/Marklin6050Interface/kernel.cpp
 *
 * Kernel supporting both binary (6050) and ASCII (6023/6223) protocols
 * with optional extension module for external event feedback.
 *
 * Copyright (C) 2025
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "kernel.hpp"
#include "../../../utils/serialport.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../log/log.hpp"
#include "../../../log/logmessageexception.hpp"

#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include <thread>
#include <chrono>
#include <vector>

using namespace Marklin6050;

// === Construction / Destruction ===

Kernel::Kernel(std::string logId_, const Config& config)
    : logId{std::move(logId_)}
    , m_config{config}
    , m_serialPort{m_ioContext}
{
}

Kernel::~Kernel()
{
    stop();
}

// === Lifecycle ===

void Kernel::start(const std::string& device, uint32_t baudrate)
{
    SerialPort::open(m_serialPort, device, baudrate, 8,
        SerialParity::None, SerialStopBits::One, SerialFlowControl::None);
}

void Kernel::stop()
{
    stopExtensionThread();
    stopInputThread();

    if(m_serialPort.is_open())
    {
        boost::system::error_code ec;
        m_serialPort.close(ec);
    }
}

// === Low-level I/O ===

bool Kernel::sendByte(uint8_t byte)
{
    if(!m_serialPort.is_open())
        return false;

    boost::system::error_code ec;
    boost::asio::write(m_serialPort, boost::asio::buffer(&byte, 1), ec);

    if(ec)
    {
        EventLoop::call(
            [this, ec]()
            {
                Log::log(logId, LogMessage::E2001_SERIAL_WRITE_FAILED_X, ec);
            });
        return false;
    }
    return true;
}

bool Kernel::sendString(const std::string& str)
{
    if(!m_serialPort.is_open())
        return false;

    boost::system::error_code ec;
    boost::asio::write(m_serialPort, boost::asio::buffer(str), ec);

    if(ec)
    {
        EventLoop::call(
            [this, ec]()
            {
                Log::log(logId, LogMessage::E2001_SERIAL_WRITE_FAILED_X, ec);
            });
        return false;
    }
    return true;
}

int Kernel::readByte()
{
    if(!m_serialPort.is_open())
        return -1;

    uint8_t byte;
    boost::system::error_code ec;
    boost::asio::read(m_serialPort, boost::asio::buffer(&byte, 1), ec);

    if(ec)
    {
        EventLoop::call(
            [this, ec]()
            {
                Log::log(logId, LogMessage::E2002_SERIAL_READ_FAILED_X, ec);
            });
        return -1;
    }
    return byte;
}

std::string Kernel::readLine()
{
    std::string line;
    while(m_serialPort.is_open())
    {
        int b = readByte();
        if(b < 0)
            return {};
        if(b == AsciiCR || b == '\n')
        {
            if(!line.empty())
                return line;
            continue;
        }
        line += static_cast<char>(b);
    }
    return {};
}

// === Protocol helpers ===

void Kernel::sendBinaryCommand(uint8_t byte1, uint8_t byte2)
{
    sendByte(byte1);
    sendByte(byte2);
}

void Kernel::sendAsciiCommand(const std::string& cmd)
{
    sendString(cmd + AsciiCR);
}

// === Redundancy helpers ===

void Kernel::sendByteWithRedundancy(uint8_t byte)
{
    sendByte(byte);

    if(m_config.redundancy > 0)
    {
        std::thread([this, byte, count = m_config.redundancy]()
        {
            for(unsigned int i = 0; i < count; i++)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                if(!m_serialPort.is_open()) return;
                sendByte(byte);
            }
        }).detach();
    }
}

void Kernel::sendBinaryCommandWithRedundancy(uint8_t byte1, uint8_t byte2)
{
    sendBinaryCommand(byte1, byte2);

    if(m_config.redundancy > 0)
    {
        std::thread([this, byte1, byte2, count = m_config.redundancy]()
        {
            for(unsigned int i = 0; i < count; i++)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                if(!m_serialPort.is_open()) return;
                sendBinaryCommand(byte1, byte2);
            }
        }).detach();
    }
}

void Kernel::sendAsciiCommandWithRedundancy(const std::string& cmd)
{
    sendAsciiCommand(cmd);

    if(m_config.redundancy > 0)
    {
        std::thread([this, cmd, count = m_config.redundancy]()
        {
            for(unsigned int i = 0; i < count; i++)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                if(!m_serialPort.is_open()) return;
                sendAsciiCommand(cmd);
            }
        }).detach();
    }
}

// === Global commands ===

bool Kernel::sendGlobalGo()
{
    if(!m_serialPort.is_open())
        return false;

    if(m_config.protocolMode == ProtocolMode::ASCII)
        sendAsciiCommandWithRedundancy("G");
    else
        sendByteWithRedundancy(GlobalGo);

    return true;
}

bool Kernel::sendGlobalStop()
{
    if(!m_serialPort.is_open())
        return false;

    if(m_config.protocolMode == ProtocolMode::ASCII)
        sendAsciiCommandWithRedundancy("S");
    else
        sendByteWithRedundancy(GlobalStop);

    return true;
}

// === Loco commands ===

void Kernel::setLocoSpeed(uint8_t address, uint8_t speed, bool f0)
{
    if(!m_serialPort.is_open() || address < 1)
        return;

    if(m_config.protocolMode == ProtocolMode::ASCII)
    {
        sendAsciiCommandWithRedundancy(
            "L " + std::to_string(address)
            + " S " + std::to_string(speed & LocoSpeedMask)
            + " F " + (f0 ? "1" : "0"));
    }
    else
    {
        uint8_t cmd = speed & LocoSpeedMask;
        if(f0)
            cmd |= LocoF0Bit;

        sendBinaryCommandWithRedundancy(cmd, address);
    }
}

void Kernel::setLocoDirection(uint8_t address, bool f0)
{
    if(!m_serialPort.is_open() || address < 1)
        return;

    // no redundancy — toggling twice cancels out
    if(m_config.protocolMode == ProtocolMode::ASCII)
    {
        sendAsciiCommand("L " + std::to_string(address) + " D");
    }
    else
    {
        uint8_t cmd = LocoDirToggle;
        if(f0)
            cmd |= LocoF0Bit;

        sendBinaryCommand(cmd, address);
    }
}

void Kernel::setLocoEmergencyStop(uint8_t address, bool f0)
{
    if(!m_serialPort.is_open() || address < 1)
        return;

    // double direction toggle: first stops, second restores direction
    if(m_config.protocolMode == ProtocolMode::ASCII)
    {
        std::string cmd = "L " + std::to_string(address) + " D";
        sendAsciiCommand(cmd);

        std::thread([this, cmd]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            if(!m_serialPort.is_open()) return;
            sendAsciiCommand(cmd);
        }).detach();
    }
    else
    {
        uint8_t cmd = LocoDirToggle;
        if(f0)
            cmd |= LocoF0Bit;

        sendBinaryCommand(cmd, address);

        std::thread([this, cmd, address]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            if(!m_serialPort.is_open()) return;
            sendBinaryCommand(cmd, address);
        }).detach();
    }
}

void Kernel::setLocoFunction(uint8_t address, uint8_t currentSpeed, bool f0)
{
    if(!m_serialPort.is_open() || address < 1)
        return;

    if(m_config.protocolMode == ProtocolMode::ASCII)
    {
        sendAsciiCommandWithRedundancy(
            "L " + std::to_string(address)
            + " S " + std::to_string(currentSpeed & LocoSpeedMask)
            + " F " + (f0 ? "1" : "0"));
    }
    else
    {
        uint8_t cmd = currentSpeed & LocoSpeedMask;
        if(f0)
            cmd |= LocoF0Bit;

        sendBinaryCommandWithRedundancy(cmd, address);
    }
}

void Kernel::setLocoFunctions1to4(uint8_t address, bool f1, bool f2, bool f3, bool f4)
{
    // F1-F4 only supported in binary mode (6050)
    if(m_config.protocolMode == ProtocolMode::ASCII)
        return;

    if(!m_serialPort.is_open() || address < 1)
        return;

    uint8_t cmd = FunctionBase;
    if(f1) cmd |= FunctionF1;
    if(f2) cmd |= FunctionF2;
    if(f3) cmd |= FunctionF3;
    if(f4) cmd |= FunctionF4;

    sendBinaryCommandWithRedundancy(cmd, address);
}

// === Accessory commands ===

bool Kernel::setAccessory(uint32_t address, OutputValue value, unsigned int timeMs)
{
    if(!m_serialPort.is_open() || address < 1 || address > 256)
        return false;

    if(m_config.protocolMode == ProtocolMode::ASCII)
    {
        // ASCII mode: CU handles timing internally
        char dir = 'G';

        std::visit([&](auto&& v)
        {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, OutputPairValue>)
                dir = (v == OutputPairValue::First) ? 'R' : 'G';
            else if constexpr (std::is_same_v<T, TriState>)
                dir = (v == TriState::True) ? 'R' : 'G';
        }, value);

        sendAsciiCommandWithRedundancy("M " + std::to_string(address) + " " + dir);
        return true;
    }
    else
    {
        // binary mode: activate → wait → deactivate cycle
        unsigned char cmd = 0;

        std::visit([&](auto&& v)
        {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, OutputPairValue>)
                cmd = (v == OutputPairValue::First) ? AccessoryRed : AccessoryGreen;
            else if constexpr (std::is_same_v<T, TriState>)
                cmd = (v == TriState::True) ? AccessoryRed : AccessoryGreen;
            else
                cmd = static_cast<unsigned char>(v);
        }, value);

        uint8_t addr = static_cast<uint8_t>(address);

        sendBinaryCommand(cmd, addr);

        std::thread([this, cmd, addr, timeMs, count = m_config.redundancy]()
        {
            // redundant activations
            for(unsigned int i = 0; i < count; i++)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                if(!m_serialPort.is_open()) return;
                sendBinaryCommand(cmd, addr);
            }

            // wait for solenoid timing
            std::this_thread::sleep_for(std::chrono::milliseconds(timeMs));
            if(!m_serialPort.is_open()) return;

            // first deactivation
            sendBinaryCommand(AccessoryOff, addr);

            // redundant deactivations
            for(unsigned int i = 0; i < count; i++)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                if(!m_serialPort.is_open()) return;
                sendBinaryCommand(AccessoryOff, addr);
            }
        }).detach();

        return true;
    }
}

// === S88 input polling ===

void Kernel::startInputThread(unsigned int moduleCount, unsigned int intervalMs)
{
    if(m_running)
        return;

    m_running = true;

    m_inputThread = std::thread([this, moduleCount, intervalMs]()
    {
        while(m_running)
        {
            if(m_config.protocolMode == ProtocolMode::ASCII)
                asciiInputLoop(moduleCount);
            else
                binaryInputLoop(moduleCount);

            std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
        }
    });
}

void Kernel::stopInputThread()
{
    if(!m_running)
        return;

    m_running = false;
    if(m_inputThread.joinable())
        m_inputThread.join();
}

void Kernel::binaryInputLoop(unsigned int modules)
{
    if(!m_running || !m_serialPort.is_open() || modules == 0)
        return;

    uint8_t cmd = S88Base + static_cast<uint8_t>(modules);

    if(!sendByte(cmd))
        return;

    const unsigned int totalBytes = modules * 2;
    std::vector<uint8_t> buffer(totalBytes);

    for(unsigned int i = 0; i < totalBytes; i++)
    {
        int b = readByte();
        if(b < 0)
            return;
        buffer[i] = static_cast<uint8_t>(b);
    }

    for(unsigned int m = 0; m < modules; m++)
    {
        uint16_t bits =
            (static_cast<uint16_t>(buffer[m * 2]) << 8) |
             static_cast<uint16_t>(buffer[m * 2 + 1]);

        for(int bit = 0; bit < 16; bit++)
        {
            bool state = bits & (1 << bit);
            uint32_t address = m * 16 + (bit + 1);

            if(s88Callback)
            {
                EventLoop::call(
                    [this, address, state]()
                    {
                        if(s88Callback)
                            s88Callback(address, state);
                    });
            }
        }
    }
}

void Kernel::asciiInputLoop(unsigned int modules)
{
    if(!m_running || !m_serialPort.is_open() || modules == 0)
        return;

    const unsigned int totalContacts = modules * 16;

    for(unsigned int contact = 1; contact <= totalContacts; contact++)
    {
        if(!m_running || !m_serialPort.is_open())
            return;

        sendAsciiCommand("C " + std::to_string(contact));

        std::string response = readLine();
        if(response.empty())
            return;

        bool state = false;
        try
        {
            state = (std::stoi(response) != 0);
        }
        catch(...)
        {
            continue;
        }

        if(s88Callback)
        {
            EventLoop::call(
                [this, contact, state]()
                {
                    if(s88Callback)
                        s88Callback(contact, state);
                });
        }
    }
}

// === Extension polling ===

void Kernel::startExtensionThread()
{
    if(m_extensionRunning)
        return;

    m_extensionRunning = true;

    m_extensionThread = std::thread([this]()
    {
        while(m_extensionRunning)
        {
            extensionPoll();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
}

void Kernel::stopExtensionThread()
{
    if(!m_extensionRunning)
        return;

    m_extensionRunning = false;
    if(m_extensionThread.joinable())
        m_extensionThread.join();
}

void Kernel::extensionPoll()
{
    if(!m_serialPort.is_open())
        return;

    // send poll command: 255 255
    if(!sendByte(Extension::PollByte) || !sendByte(Extension::PollByte))
        return;

    // read event count
    int countByte = readByte();
    if(countByte <= 0)
        return;

    uint8_t count = static_cast<uint8_t>(countByte);

    for(uint8_t i = 0; i < count; i++)
    {
        if(!m_serialPort.is_open())
            return;

        // read event type
        int typeByte = readByte();
        if(typeByte < 0)
            return;

        switch(static_cast<uint8_t>(typeByte))
        {
            case Extension::EventGlobal:
            {
                int data = readByte();
                if(data < 0)
                    return;

                bool power = data & Extension::GlobalPowerBit;
                bool run = data & Extension::GlobalRunBit;

                if(extensionGlobalCallback)
                {
                    EventLoop::call(
                        [this, power, run]()
                        {
                            if(extensionGlobalCallback)
                                extensionGlobalCallback(power, run);
                        });
                }
                break;
            }
            case Extension::EventTurnout:
            {
                int addrByte = readByte();
                int stateByte = readByte();
                if(addrByte < 0 || stateByte < 0)
                    return;

                // address 0 means 256
                uint32_t address = (addrByte == 0) ? 256 : static_cast<uint32_t>(addrByte);
                bool green = (stateByte != 0);

                if(extensionTurnoutCallback)
                {
                    EventLoop::call(
                        [this, address, green]()
                        {
                            if(extensionTurnoutCallback)
                                extensionTurnoutCallback(address, green);
                        });
                }
                break;
            }
            case Extension::EventLocoState:
            {
                int addrByte = readByte();
                int dataByte = readByte();
                if(addrByte < 0 || dataByte < 0)
                    return;

                uint8_t address = static_cast<uint8_t>(addrByte);
                uint8_t speed = dataByte & Extension::LocoSpeedBits;
                bool f0 = dataByte & Extension::LocoF0Bit_Ext;
                bool forward = !(dataByte & Extension::LocoDirBit);

                if(extensionLocoCallback)
                {
                    EventLoop::call(
                        [this, address, speed, f0, forward]()
                        {
                            if(extensionLocoCallback)
                                extensionLocoCallback(address, speed, f0, forward);
                        });
                }
                break;
            }
            case Extension::EventLocoFunc:
            {
                int addrByte = readByte();
                int dataByte = readByte();
                if(addrByte < 0 || dataByte < 0)
                    return;

                uint8_t address = static_cast<uint8_t>(addrByte);
                bool f1 = dataByte & Extension::LocoF1Bit;
                bool f2 = dataByte & Extension::LocoF2Bit;
                bool f3 = dataByte & Extension::LocoF3Bit;
                bool f4 = dataByte & Extension::LocoF4Bit;

                if(extensionFuncCallback)
                {
                    EventLoop::call(
                        [this, address, f1, f2, f3, f4]()
                        {
                            if(extensionFuncCallback)
                                extensionFuncCallback(address, f1, f2, f3, f4);
                        });
                }
                break;
            }
            default:
                // unknown event type — can't know how many bytes to skip
                // bail out to avoid reading garbage
                return;
        }
    }
}
