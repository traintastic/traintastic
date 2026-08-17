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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_RCLINK_RCLINKKERNEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_RCLINK_RCLINKKERNEL_HPP

#include "../kernelbase.hpp"
#include <span>
#include "rclinkconfig.hpp"
#include "iohandler/rclinkiohandler.hpp"

namespace RCLink {

class Kernel : public ::KernelBase
{
public:
  static constexpr uint8_t inputAddressMin = 1;
  static constexpr uint8_t inputAddressMax = 24;

  std::function<void(uint8_t, bool, uint16_t, bool)> onDetector;

  /**
   * @brief Create kernel and IO handler
   *
   * @param[in] config RCLink configuration
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
   * @brief Set RCLink configuration
   *
   * @param[in] config The RCLink configuration
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

  void receive(std::span<const uint8_t> message);

private:
  std::unique_ptr<IOHandler> m_ioHandler;
  const bool m_simulation;
  Config m_config;
  std::array<uint16_t, 24> m_railcomAddress;

  Kernel(std::string logId_, const Config& config, bool simulation);

  Kernel(const Kernel&) = delete;
  Kernel& operator =(const Kernel&) = delete;

  void setIOHandler(std::unique_ptr<IOHandler> handler);

  template<typename T>
  void send(const T& message);
  void send(std::span<const uint8_t> message);
};

}

#endif
