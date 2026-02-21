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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_CBUSKERNEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CBUS_CBUSKERNEL_HPP

#include "../kernelbase.hpp"
#include <span>
#include <traintastic/enum/direction.hpp>
#include "cbusconfig.hpp"
#include "iohandler/cbusiohandler.hpp"

namespace CBUS {

class Kernel : public ::KernelBase
{
public:
  std::function<void()> onTrackOff;
  std::function<void()> onTrackOn;
  std::function<void()> onEmergencyStop;

  /**
   * @brief Create kernel and IO handler
   *
   * @param[in] config CBUS configuration
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
   * @brief Set CBUS configuration
   *
   * @param[in] config The CBUS configuration
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

  void receive(uint8_t canId, const Message& message);

  void trackOff();
  void trackOn();
  void requestEmergencyStop();

  bool send(std::vector<uint8_t> message);
  bool sendDCC(std::vector<uint8_t> dccPacket, uint8_t repeat);

private:
  std::unique_ptr<IOHandler> m_ioHandler;
  const bool m_simulation;
  Config m_config;
  bool m_trackOn = false;

  Kernel(std::string logId_, const Config& config, bool simulation);

  Kernel(const Kernel&) = delete;
  Kernel& operator =(const Kernel&) = delete;

  void setIOHandler(std::unique_ptr<IOHandler> handler);

  void send(const Message& message);
};

}

#endif
