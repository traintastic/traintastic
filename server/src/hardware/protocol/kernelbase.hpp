/**
 * server/src/hardware/protocol/kernelbase.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023-2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_KERNELBASE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_KERNELBASE_HPP

#include <string>
#include <functional>
#include <thread>
#include <boost/asio/io_context.hpp>

class KernelBase
{
  private:
    std::function<void()> m_onStarted;
    std::function<void()> m_onError;

  protected:
    boost::asio::io_context m_ioContext;
    std::thread m_thread;

#ifndef NDEBUG
    bool m_started = false;
#endif

    KernelBase(std::string logId_);

    virtual void started();

  public:
    virtual ~KernelBase() = default;

    const std::string logId; //!< Object id for log messages.

    /**
     * \brief IO context for LocoNet kernel and IO handler
     *
     * \return The IO context
     */
    boost::asio::io_context& ioContext()
    {
      return m_ioContext;
    }

    /**
     * \brief ...
     *
     * \param[in] callback Handler to call when the kernel is fully started.
     * \note This function may not be called when the kernel is running.
     */
    void setOnStarted(std::function<void()> callback);

    /**
     * \brief Register error handler
     *
     * Once this handler is called the LocoNet communication it stopped.
     *
     * \param[in] callback Handler to call in case of an error.
     * \note This function may not be called when the kernel is running.
     */
    void setOnError(std::function<void()> callback);

    /**
     * \brief Report fatal error
     * Must be called by the IO handler in case of a fatal error.
     * This will put the interface in error state.
     * \note This function must run in the event loop thread.
     */
    void error();
};

#endif
