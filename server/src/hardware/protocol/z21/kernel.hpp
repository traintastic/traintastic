/**
 * server/src/hardware/protocol/z21/kernel.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_Z21_KERNEL_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_Z21_KERNEL_HPP

#include <array>
#include <thread>
#include <boost/asio/io_context.hpp>

#include "config.hpp"
#include "iohandler/iohandler.hpp"

class Decoder;
enum class DecoderChangeFlags;
class DecoderController;

namespace Z21 {

struct Message;
enum HardwareType : uint32_t;

class Kernel
{
  private:
    std::thread m_thread;
    std::function<void()> m_onStarted;

  protected:
    boost::asio::io_context m_ioContext;
    std::unique_ptr<IOHandler> m_ioHandler;
    std::string m_logId;
#ifndef NDEBUG
    bool m_started;
#endif

    Kernel();
    virtual ~Kernel() = default;

    void setIOHandler(std::unique_ptr<IOHandler> handler);

    virtual void onStart() {}
    virtual void onStop() {}

  public:
    Kernel(const Kernel&) = delete;
    Kernel& operator =(const Kernel&) = delete;

    /**
     * @brief IO context for Z21 kernel and IO handler
     *
     * @return The IO context
     */
    boost::asio::io_context& ioContext() { return m_ioContext; }

    /**
     * @brief Get object id used for log messages
     * @return The object id
     */
    inline const std::string& logId()
    {
      return m_logId;
    }

    /**
     * @brief Set object id used for log messages
     * @param[in] value The object id
     */
    inline void setLogId(std::string value)
    {
      m_logId = std::move(value);
    }

    /**
     * @brief ...
     * @param[in] callback ...
     * @note This function may not be called when the kernel is running.
     */
    inline void setOnStarted(std::function<void()> callback)
    {
      assert(!m_started);
      m_onStarted = std::move(callback);
    }

    /**
     * @brief Start the kernel and IO handler
     */
    virtual void start();

    /**
     * @brief Stop the kernel and IO handler
     */
    virtual void stop();
};

}

#endif
