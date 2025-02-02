/**
 * server/src/hardware/protocol/selectrix/kernel.hpp
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_SELECTRIX_KERNEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_SELECTRIX_KERNEL_HPP

#include "../kernelbase.hpp"
#include <map>
#include <boost/asio/steady_timer.hpp>
#include <traintastic/enum/tristate.hpp>
#include "addresstype.hpp"
#include "bus.hpp"
#include "config.hpp"
#include "iohandler/iohandler.hpp"

class Decoder;
enum class DecoderChangeFlags;
class DecoderController;
enum class SimulateInputAction;
class InputController;

namespace Selectrix {

class Kernel : public ::KernelBase
{
  private:
    struct BusAddressValue
    {
      AddressType type;
      uint8_t lastValue;
      bool lastValueValid;
    };

    std::unique_ptr<IOHandler> m_ioHandler;
    const bool m_simulation;

    TriState m_trackPower;
    std::function<void(bool)> m_onTrackPowerChanged;

    std::array<boost::asio::steady_timer, addressTypes.size()> m_pollTimer;
    std::array<std::chrono::time_point<std::chrono::steady_clock>, addressTypes.size()> m_nextPoll;
    std::map<BusAddress, BusAddressValue> m_addresses;

    DecoderController* m_decoderController = nullptr;

    InputController* m_inputController = nullptr;

    Config m_config;

    Kernel(std::string logId_, const Config& config, bool simulation);

    void setIOHandler(std::unique_ptr<IOHandler> handler);

    bool read(Bus bus, uint8_t address);
    bool write(Bus bus, uint8_t address, uint8_t value);

    inline void postWrite(Bus bus, uint8_t address, uint8_t value)
    {
      m_ioContext.post(
        [this, bus, address, value]()
        {
          write(bus, address, value);
        });
    }

    void startPollTimer(AddressType addressType);
    void poll(const boost::system::error_code& ec, AddressType addressType);

  public:
    Kernel(const Kernel&) = delete;
    Kernel& operator =(const Kernel&) = delete;
    ~Kernel();

  #ifndef NDEBUG
    bool isKernelThread() const
    {
      return std::this_thread::get_id() == m_thread.get_id();
    }
  #endif

    /**
     * \brief Create kernel and IO handler
     *
     * \param[in] config Selectrix configuration
     * \param[in] args IO handler arguments
     * \return The kernel instance
     */
    template<class IOHandlerType, class... Args>
    static std::unique_ptr<Kernel> create(std::string logId_, const Config& config, Args... args)
    {
      static_assert(std::is_base_of_v<IOHandler, IOHandlerType>);
      std::unique_ptr<Kernel> kernel{new Kernel(std::move(logId_), config, isSimulation<IOHandlerType>())};
      kernel->setIOHandler(std::make_unique<IOHandlerType>(*kernel, std::forward<Args>(args)...));
      return kernel;
    }

    /**
     * \brief Set LocoNet configuration
     *
     * \param[in] config The Selectrix configuration
     */
    void setConfig(const Config& config);

    /**
     * \brief ...
     *
     * \param[in] callback ...
     * \note This function may not be called when the kernel is running.
     */
    void setOnTrackPowerChanged(std::function<void(bool)> callback);

    /**
     * \brief Set the decoder controller
     *
     * \param[in] decoderController The decoder controller
     * \note This function may not be called when the kernel is running.
     */
    void setDecoderController(DecoderController* decoderController);

    /**
     * \brief Set the input controller
     *
     * \param[in] inputController The input controller
     * \note This function may not be called when the kernel is running.
     */
    void setInputController(InputController* inputController);

    /**
     * \brief Start the kernel and IO handler
     */
    void start();

    /**
     * \brief Stop the kernel and IO handler
     */
    void stop();

    /**
     * \brief ...
     *
     * This must be called by the IO handler whenever a bus value changes.
     *
     * \note This function must run in the kernel's IO context
     */
    void busChanged(Bus bus, uint8_t address, uint8_t value);

    /**
     *
     *
     */
    void setTrackPower(bool value);

    /**
     *
     *
     */
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber);

    /**
     * \brief Simulate input change
     * \param[in] bus SX-bus.
     * \param[in] address Input address
     * \param[in] action Simulation action to perform
     */
    void simulateInputChange(Bus bus, uint16_t address, SimulateInputAction action);

    void addAddress(Bus bus, uint8_t address, AddressType type);
    void removeAddress(Bus bus, uint8_t address);
};

}

#endif
