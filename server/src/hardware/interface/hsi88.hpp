/**
 * server/src/hardware/interface/hsi88.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_HSI88_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_HSI88_HPP

#include "interface.hpp"
#include <thread>
#include <string_view>
#include <vector>
#include <queue>
#include <boost/asio/io_context.hpp>
#include <boost/asio/serial_port.hpp>
#include "../../core/serialdeviceproperty.hpp"
#include "../input/inputcontroller.hpp"

/**
 * \brief HSI-88 hardware interface
 */
class HSI88Interface final
  : public Interface
  , public InputController
{
  CLASS_ID("interface.hsi88")
  DEFAULT_ID("hsi88")
  CREATE(HSI88Interface)

  private:
    static constexpr uint8_t modulesMin = 0;
    static constexpr uint8_t modulesMax = 31;
    static constexpr uint8_t modulesTotal = 31;
    static constexpr uint8_t inputsPerModule = 16;
    static constexpr uint32_t inputAddressMin = 1;
    static constexpr uint32_t inputAddressMax = modulesMax * 16;

    struct InputChannel
    {
      static constexpr uint32_t left = 1;
      static constexpr uint32_t middle = 2;
      static constexpr uint32_t right = 3;
    };

    inline static const std::vector<uint32_t> channels = {
      InputChannel::left,
      InputChannel::middle,
      InputChannel::right,
    };

    inline static const std::vector<std::string_view> channelNames = {
      "$hsi88:s88_left$",
      "$hsi88:s88_middle$",
      "$hsi88:s88_right$",
    };

    static constexpr std::string_view versionInquiry = "v\r";
    static constexpr std::string_view terminalModeOff = "t0\r";

    boost::asio::io_context m_ioContext;
    std::thread m_thread;
    boost::asio::serial_port m_serialPort;
    std::array<char, 128> m_readBuffer;
    size_t m_readBufferOffset = 0;
    std::queue<std::string> m_sendQueue;
    std::array<char, 16> m_writeBuffer;
    size_t m_writeBufferOffset = 0;
    bool m_waitingForReply = false;
    bool m_simulation = false;
    std::vector<TriState> m_inputValues;
    std::atomic<bool> m_debugLogRXTX;

#ifndef NDEBUG
    inline bool isHSI88Thread() const { return std::this_thread::get_id() == m_thread.get_id(); }
#endif

    void read();
    void write();

    void receive(std::string_view message);
    void send(std::string message);
    void sendNext();

    void updateModulesMax();

  protected:
    void addToWorld() final;
    void destroying() final;
    void loaded() final;
    void worldEvent(WorldState state, WorldEvent event) final;
    bool setOnline(bool& value, bool simulation) final;

  public:
    SerialDeviceProperty device;
    Property<uint8_t> modulesLeft;
    Property<uint8_t> modulesMiddle;
    Property<uint8_t> modulesRight;
    Property<bool> debugLogRXTX;

    HSI88Interface(World& world, std::string_view _id);

    // InputController:
    const std::vector<uint32_t>* inputChannels() const final { return &channels; }
    const std::vector<std::string_view>* inputChannelNames() const final { return &channelNames; }
    std::pair<uint32_t, uint32_t> inputAddressMinMax(uint32_t channel) const final;
    void inputSimulateChange(uint32_t channel, uint32_t address, SimulateInputAction action) final;
};

#endif
