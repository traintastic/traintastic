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

#include <memory>
#include <span>
#include <cstddef>
#include <boost/asio/ip/tcp.hpp>

namespace CAN {

struct Message;
class IOHub;

class IOHubConnection : public std::enable_shared_from_this<IOHubConnection>
{
public:
  ~IOHubConnection();

  void start();
  void stop();

  void send(const Message& message);

protected:
  IOHubConnection(std::shared_ptr<IOHub> hub, boost::asio::ip::tcp::socket socket);

  virtual size_t deserialize(std::span<const std::byte> buffer, CAN::Message& message, bool& haveMessage) = 0;
  virtual size_t serialize(const Message& message, std::span<std::byte> buffer) = 0;

private:
  std::shared_ptr<IOHub> m_hub;
  boost::asio::ip::tcp::socket m_socket;
  std::array<std::byte, 1024> m_readBuffer;
  size_t m_readBufferOffset = 0;
  std::array<std::byte, 1024> m_writeBuffer;
  size_t m_writeBufferOffset = 0;

  void read();
  void write();
  void disconnect();
};

}
