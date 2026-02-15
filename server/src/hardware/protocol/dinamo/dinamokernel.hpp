/**
 * This file is part of Traintastic,
 * see <https://github.com/traintastic/traintastic>.
 *
 * Copyright (C) 2026 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DINAMO_DINAMOKERNEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_DINAMO_DINAMOKERNEL_HPP

#include "../kernelbase.hpp"
#include <span>
#include <traintastic/enum/direction.hpp>
#include "dinamoconfig.hpp"
#include "dinamopolarity.hpp"
#include "dinamotypes.hpp"
#include "iohandler/dinamoiohandler.hpp"

namespace Dinamo {

class Kernel : public ::KernelBase
{
public:
  static constexpr uint16_t inputAddressMin = 0;
  static constexpr uint16_t inputAddressMax = 2047;
  static constexpr uint16_t outputOC32AddressMin = 0;
  static constexpr uint16_t outputOC32AddressMax = (32 * 32) - 1; //!< 32 modules, 32 outputs per module
  static constexpr uint8_t blockAddressMin = 0;
  static constexpr uint8_t blockAddressMax = 255;

  std::function<void()> onFault;
  std::function<void(uint16_t, bool)> onInputChanged;

  /**
   * @brief Create kernel and IO handler
   *
   * @param[in] config Dinamo configuration
   * @param[in] args IO handler arguments
   * @return The kernel instance
   */
  template<class IOHandlerType, class... Args>
  static std::unique_ptr<Kernel> create(std::string logId_, const Config& config, Args... args)
  {
    static_assert(std::is_base_of_v<IOHandler, IOHandlerType>);
    std::unique_ptr<Kernel> kernel{new Kernel(std::move(logId_), config, isSimulation<IOHandlerType>())};
    kernel->setIOHandler(std::make_unique<IOHandlerType>(*kernel, std::forward<Args>(args)...));
    return kernel;
  }

#ifndef NDEBUG
  bool isKernelThread() const
  {
    return std::this_thread::get_id() == m_thread.get_id();
  }
#endif

  /**
   * @brief Set DINAMO configuration
   *
   * @param[in] config The DINAMO configuration
   */
  void setConfig(const Config& config);

  /**
   * @brief Start the kernel and IO handler
   */
  void start();

  /**
   * @brief Stop the kernel and IO handler
   */
  void stop();

  /**
   * \brief Notify kernel the IO handler is started.
   * \note This function must run in the kernel's IO context
   */
  void started() final;

  void receive(std::span<const uint8_t> message, bool hold, bool fault);

  void setFault();
  void resetFault();

  void requestInputState(std::vector<uint16_t> inputAddresses);

  void setOC32Aspect(uint16_t address, uint8_t aspect);

  void setBlockAnalog(uint8_t block, bool light, std::optional<Polarity> polarity = std::nullopt);
  void setBlockAnalogSpeed(uint8_t block, uint8_t speed, std::optional<Dinamo::Polarity> polarity = std::nullopt);
  void setBlockAnalogLight(uint8_t block, bool on);

  void setBlockDCC(uint8_t block, std::optional<Polarity> polarity = std::nullopt);
  void setBlockDCCSpeedAndDirection(uint8_t block, uint16_t address, bool longAddress, bool emergencyStop, uint8_t speedStep, Direction direction);
  void setBlockDCCFunctionF0F4(uint8_t block, uint16_t address, bool longAddress, bool f0, bool f1, bool f2, bool f3, bool f4);
  void setBlockDCCFunctionF5F8(uint8_t block, uint16_t address, bool longAddress, bool f5, bool f6, bool f7, bool f8);
  void setBlockDCCFunctionF9F12(uint8_t block, uint16_t address, bool longAddress, bool f9, bool f10, bool f11, bool f12);
  void setBlockDCCFunctions(uint8_t block, uint16_t address, bool longAddress, uint32_t functions);

  void linkBlock(uint8_t destinationBlock, uint8_t sourceBlock, bool invertPolarity);
  void unlinkBlockUp(uint8_t block);
  void unlinkBlockDown(uint8_t block);
  void resetBlock(uint8_t block);

private:
  std::unique_ptr<IOHandler> m_ioHandler;
  const bool m_simulation;
  Config m_config;
  Version m_protocolVersion;
  System m_system;
  bool m_rxFault = true;
  bool m_txFault = true;

  Kernel(std::string logId_, const Config& config, bool simulation);

  Kernel(const Kernel&) = delete;
  Kernel& operator =(const Kernel&) = delete;

  void setIOHandler(std::unique_ptr<IOHandler> handler);

  template<typename T>
  void send(const T& message);
  void send(std::span<const uint8_t> message);

  void handleSystemCommand(std::span<const uint8_t> message);
  void handleInput(std::span<const uint8_t> message);
};

}

#endif
