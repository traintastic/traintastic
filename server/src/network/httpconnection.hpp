/**
 * server/src/network/httpconnection.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2024 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_NETWORK_HTTPCONNECTION_HPP
#define TRAINTASTIC_SERVER_NETWORK_HTTPCONNECTION_HPP

#include <memory>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>

class Server;

class HTTPConnection : public std::enable_shared_from_this<HTTPConnection>
{
private:
  std::shared_ptr<Server> m_server;
  boost::beast::tcp_stream m_stream;
  boost::beast::flat_buffer m_buffer;
  boost::beast::http::request<boost::beast::http::string_body> m_request;

  void doRead();
  void doClose();

public:
  HTTPConnection(std::shared_ptr<Server> server, boost::asio::ip::tcp::socket&& socket);

  void start();
};

#endif
