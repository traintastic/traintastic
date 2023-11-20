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
#include <traintastic/enum/tristate.hpp>
#include "bus.hpp"
#include "config.hpp"
#include "iohandler/iohandler.hpp"

namespace Selectrix {

struct Message;

class Kernel : public ::KernelBase
{
  private:
    std::unique_ptr<IOHandler> m_ioHandler;
    const bool m_simulation;
    Bus m_bus = Bus::SX0;

    TriState m_trackPower;
    std::function<void(bool)> m_onTrackPowerChanged;

    Config m_config;

    Kernel(std::string logId_, const Config& config, bool simulation);

    void setIOHandler(std::unique_ptr<IOHandler> handler);

    bool selectBus(Bus bus);
    bool read(Bus bus, uint8_t address, uint8_t& value);
    bool write(Bus bus, uint8_t address, uint8_t value);
    bool write(uint8_t address, uint8_t value);

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
     * \brief Start the kernel and IO handler
     */
    void start();

    /**
     * \brief Stop the kernel and IO handler
     */
    void stop();

    /**
     *
     *
     */
    void setTrackPower(bool value);
};

}

#endif
