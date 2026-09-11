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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CAN_IOHUB_CANIOHUB_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_CAN_IOHUB_CANIOHUB_HPP

#include <memory>
#include <list>
#include <boost/asio/ip/tcp.hpp>

namespace CAN {

struct Message;
class IOHubConnection;

class IOHub : public std::enable_shared_from_this<IOHub>
{
  friend class IOHubConnection;

public:
  using OnReceive = std::function<void(const Message& message)>;

  virtual ~IOHub();

  void start(OnReceive onReceive);
  void stop();

  void send(const Message& message);

protected:
  IOHub(boost::asio::io_context& ioContext, std::string logId, bool localhostOnly, uint16_t port);

  virtual std::shared_ptr<IOHubConnection> newConnection(boost::asio::ip::tcp::socket socket) = 0;

private:
  boost::asio::ip::tcp::acceptor m_acceptor;
  const std::string m_logId;
  const bool m_localhostOnly;
  const uint16_t m_port;
  OnReceive m_onReceive;
  std::list<std::shared_ptr<IOHubConnection>> m_connections;

  void accept();
  void receive(const Message& message, IOHubConnection& source);
};

}

#endif
