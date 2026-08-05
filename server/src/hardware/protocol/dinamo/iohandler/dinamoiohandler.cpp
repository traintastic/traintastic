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

#include "dinamoiohandler.hpp"
#include "../dinamokernel.hpp"
#include "../../../../core/eventloop.hpp"
#include "../../../../log/log.hpp"

namespace {

using namespace std::chrono_literals;

constexpr auto idleTimeout = 5ms;

}

namespace Dinamo {

IOHandler::IOHandler(Kernel& kernel)
  : m_kernel{kernel}
  , m_timer{m_kernel.ioContext()}
{
}

void IOHandler::start()
{
  sendNull();
  m_kernel.started();
}

void IOHandler::sendNull()
{
  if(auto ec = send({}, m_txHold, m_txFault)) [[unlikely]]
  {
    EventLoop::call(
      [this, ec]()
      {
        Log::log(m_kernel.logId, LogMessage::E2001_SERIAL_WRITE_FAILED_X, ec);
        m_kernel.error();
      });
  }
}

void IOHandler::startIdleTimeoutTimer()
{
  m_timer.cancel();
  m_timer.expires_from_now(idleTimeout);
  m_timer.async_wait(
    [this](const boost::system::error_code& ec)
    {
      if(!ec) // no message to send, sent Null
      {
        sendNull();
      }
    });
}

}
