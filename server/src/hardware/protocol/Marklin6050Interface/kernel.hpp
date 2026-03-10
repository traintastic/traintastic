/**
 * server/src/hardware/protocol/Marklin6050Interface/kernel.hpp
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

#pragma once

#include <string>
#include <cstdint>
#include <thread>
#include <functional>
#include <atomic>
#include <boost/asio/io_context.hpp>
#include <boost/asio/serial_port.hpp>

#include "protocol.hpp"
#include "config.hpp"
#include "../../output/outputvalue.hpp"

namespace Marklin6050 {

class Kernel
{
public:
    const std::string logId;

    Kernel(std::string logId, const Config& config);
    ~Kernel();

    // --- Lifecycle ---
    void start(const std::string& device, uint32_t baudrate);
    void stop();
    bool isRunning() const { return m_running.load(); }

    // --- Loco commands ---
    void setLocoSpeed(uint8_t address, uint8_t speed, bool f0);
    void setLocoDirection(uint8_t address, bool f0);
    void setLocoEmergencyStop(uint8_t address, bool f0);
    void setLocoFunction(uint8_t address, uint8_t currentSpeed, bool f0);
    void setLocoFunctions1to4(uint8_t address, bool f1, bool f2, bool f3, bool f4);

    // --- Accessory commands ---
    bool setAccessory(uint32_t address, OutputValue value, unsigned int timeMs);

    // --- Global commands ---
    bool sendGlobalGo();
    bool sendGlobalStop();

    // --- S88 input polling ---
    void startInputThread(unsigned int moduleCount, unsigned int intervalMs);
    void stopInputThread();

    // --- Extension polling ---
    void startExtensionThread();
    void stopExtensionThread();

    // --- Callbacks ---
    std::function<void(uint32_t, bool)> s88Callback;
    std::function<void(bool power, bool run)> extensionGlobalCallback;
    std::function<void(uint32_t address, bool green)> extensionTurnoutCallback;
    std::function<void(uint8_t address, uint8_t speed, bool f0, bool forward)> extensionLocoCallback;
    std::function<void(uint8_t address, bool f1, bool f2, bool f3, bool f4)> extensionFuncCallback;

private:
    Config m_config;
    boost::asio::io_context m_ioContext;
    boost::asio::serial_port m_serialPort;
    std::thread m_inputThread;
    std::thread m_extensionThread;
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_extensionRunning{false};

    // --- Low-level I/O ---
    bool sendByte(uint8_t byte);
    bool sendString(const std::string& str);
    int readByte();
    std::string readLine();

    // --- Protocol helpers ---
    void sendBinaryCommand(uint8_t byte1, uint8_t byte2);
    void sendAsciiCommand(const std::string& cmd);

    // --- Redundancy helpers ---
    void sendByteWithRedundancy(uint8_t byte);
    void sendBinaryCommandWithRedundancy(uint8_t byte1, uint8_t byte2);
    void sendAsciiCommandWithRedundancy(const std::string& cmd);

    // --- S88 polling ---
    void binaryInputLoop(unsigned int modules);
    void asciiInputLoop(unsigned int modules);

    // --- Extension polling ---
    void extensionPoll();
};

} // namespace Marklin6050
