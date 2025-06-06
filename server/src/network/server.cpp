/**
 * server/src/network/server.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022-2025 Reinder Feenstra
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

#include "server.hpp"
#include <boost/beast/http/buffer_body.hpp>
#include <span>
#include <traintastic/network/message.hpp>
#include <version.hpp>
#include "clientconnection.hpp"
#include "httpconnection.hpp"
#include "webthrottleconnection.hpp"
#include "../core/eventloop.hpp"
#include "../log/log.hpp"
#include "../log/logmessageexception.hpp"
#include "../utils/setthreadname.hpp"

//#define SERVE_FROM_FS // Development option, NOT for production!
#ifdef SERVE_FROM_FS
  #include "../utils/readfile.hpp"

  static const auto www = std::filesystem::absolute(std::filesystem::path(__FILE__).parent_path() / ".." / ".." / "www");
#else
  #include <resource/www/throttle.html.hpp>
  #include <resource/www/css/throttle.css.hpp>
  #include <resource/www/js/throttle.js.hpp>
#endif
#include <resource/www/css/normalize.css.hpp>
#include <resource/shared/gfx/appicon.ico.hpp>

#define IS_SERVER_THREAD (std::this_thread::get_id() == m_thread.get_id())

namespace beast = boost::beast;
namespace http = beast::http;

namespace
{

static constexpr std::string_view serverHeader{"Traintastic-server/" TRAINTASTIC_VERSION_FULL};
static constexpr std::string_view contentTypeTextPlain{"text/plain"};
static constexpr std::string_view contentTypeTextHtml{"text/html"};
static constexpr std::string_view contentTypeTextCss{"text/css"};
static constexpr std::string_view contentTypeTextJavaScript{"text/javascript"};
static constexpr std::string_view contentTypeImageXIcon{"image/x-icon"};

http::message_generator notFound(const http::request<http::string_body>& request)
{
  http::response<http::string_body> response{http::status::not_found, request.version()};
  response.set(http::field::server, serverHeader);
  response.set(http::field::content_type, contentTypeTextPlain);
  response.keep_alive(request.keep_alive());
  response.body() = "404 Not Found";
  response.prepare_payload();
  return response;
}

http::message_generator methodNotAllowed(const http::request<http::string_body>& request, std::initializer_list<http::verb> allowedMethods)
{
  std::string allow;
  for(auto method : allowedMethods)
  {
    allow.append(http::to_string(method)).append(" ");
  }
  http::response<http::string_body> response{http::status::method_not_allowed, request.version()};
  response.set(http::field::server, serverHeader);
  response.set(http::field::content_type, contentTypeTextPlain);
  response.set(http::field::allow, allow);
  response.keep_alive(request.keep_alive());
  response.body() = "405 Method Not Allowed";
  response.prepare_payload();
  return response;
}

http::message_generator binary(const http::request<http::string_body>& request, std::string_view contentType, std::span<const std::byte> body)
{
  if(request.method() != http::verb::get && request.method() != http::verb::head)
  {
    return methodNotAllowed(request, {http::verb::get, http::verb::head});
  }
  http::response<http::buffer_body> response{http::status::ok, request.version()};
  response.set(http::field::server, serverHeader);
  response.set(http::field::content_type, contentType);
  response.keep_alive(request.keep_alive());
  if(request.method() == http::verb::head)
  {
    response.content_length(body.size());
  }
  else
  {
    response.body().data = const_cast<std::byte*>(body.data());
    response.body().size = body.size();
  }
  response.body().more = false;
  response.prepare_payload();
  return response;
}

http::message_generator text(const http::request<http::string_body>& request, std::string_view contentType, std::string_view body)
{
  if(request.method() != http::verb::get && request.method() != http::verb::head)
  {
    return methodNotAllowed(request, {http::verb::get, http::verb::head});
  }
  http::response<http::string_body> response{http::status::ok, request.version()};
  response.set(http::field::server, serverHeader);
  response.set(http::field::content_type, contentType);
  response.keep_alive(request.keep_alive());
  if(request.method() == http::verb::head)
  {
    response.content_length(body.size());
  }
  else
  {
    response.body() = body;
  }
  response.prepare_payload();
  return response;
}

http::message_generator textPlain(const http::request<http::string_body>& request, std::string_view body)
{
  return text(request, contentTypeTextPlain, body);
}

http::message_generator textHtml(const http::request<http::string_body>& request, std::string_view body)
{
  return text(request, contentTypeTextHtml, body);
}

http::message_generator textCss(const http::request<http::string_body>& request, std::string_view body)
{
  return text(request, contentTypeTextCss, body);
}

http::message_generator textJavaScript(const http::request<http::string_body>& request, std::string_view body)
{
  return text(request, contentTypeTextJavaScript, body);
}

}

Server::Server(bool localhostOnly, uint16_t port, bool discoverable)
  : m_ioContext{1}
  , m_acceptor{m_ioContext}
  , m_socketUDP{m_ioContext}
  , m_localhostOnly{localhostOnly}
{
  assert(isEventLoopThread());

  boost::system::error_code ec;
  boost::asio::ip::tcp::endpoint endpoint(localhostOnly ? boost::asio::ip::address_v4::loopback() : boost::asio::ip::address_v4::any(), port);

  m_acceptor.open(endpoint.protocol(), ec);
  if(ec)
    throw LogMessageException(LogMessage::F1001_OPENING_TCP_SOCKET_FAILED_X, ec);

  m_acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
  if(ec)
    throw LogMessageException(LogMessage::F1002_TCP_SOCKET_ADDRESS_REUSE_FAILED_X, ec);

  m_acceptor.bind(endpoint, ec);
  if(ec)
    throw LogMessageException(LogMessage::F1003_BINDING_TCP_SOCKET_FAILED_X, ec);

  m_acceptor.listen(5, ec);
  if(ec)
    throw LogMessageException(LogMessage::F1004_TCP_SOCKET_LISTEN_FAILED_X, ec);

  if(discoverable)
  {
    if(port == defaultPort)
    {
      m_socketUDP.open(boost::asio::ip::udp::v4(), ec);
      if(ec)
        throw LogMessageException(LogMessage::F1005_OPENING_UDP_SOCKET_FAILED_X, ec);

      m_socketUDP.set_option(boost::asio::socket_base::reuse_address(true), ec);
      if(ec)
        throw LogMessageException(LogMessage::F1006_UDP_SOCKET_ADDRESS_REUSE_FAILED_X, ec);

      m_socketUDP.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::any(), defaultPort), ec);
      if(ec)
        throw LogMessageException(LogMessage::F1007_BINDING_UDP_SOCKET_FAILED_X, ec);

      Log::log(id, LogMessage::N1005_DISCOVERY_ENABLED);
    }
    else
    {
      Log::log(id, LogMessage::W1001_DISCOVERY_DISABLED_ONLY_ALLOWED_ON_PORT_X, defaultPort);
      discoverable = false;
    }
  }
  else
    Log::log(id, LogMessage::N1006_DISCOVERY_DISABLED);

  Log::log(id, LogMessage::N1007_LISTENING_AT_X_X, m_acceptor.local_endpoint().address().to_string(), m_acceptor.local_endpoint().port());

  m_thread = std::thread(
    [this]()
    {
      setThreadName("server");
      auto work = std::make_shared<boost::asio::io_context::work>(m_ioContext);
      m_ioContext.run();
    });

  m_ioContext.post(
    [this, discoverable]()
    {
      if(discoverable)
        doReceive();

      doAccept();
    });
}

Server::~Server()
{
  assert(isEventLoopThread());

  if(!m_ioContext.stopped())
  {
    m_ioContext.post(
      [this]()
      {
        boost::system::error_code ec;
        if(m_acceptor.cancel(ec))
          Log::log(id, LogMessage::E1008_SOCKET_ACCEPTOR_CANCEL_FAILED_X, ec);

        m_acceptor.close();

        m_socketUDP.close();
      });

    m_ioContext.stop();
  }

  if(m_thread.joinable())
    m_thread.join();

  while(!m_connections.empty())
    m_connections.front()->disconnect();
}

void Server::connectionGone(const std::shared_ptr<WebSocketConnection>& connection)
{
  assert(isEventLoopThread());

  m_connections.erase(std::find(m_connections.begin(), m_connections.end(), connection));
}

void Server::doReceive()
{
  assert(IS_SERVER_THREAD);

  m_socketUDP.async_receive_from(boost::asio::buffer(m_udpBuffer), m_remoteEndpoint,
    [this](const boost::system::error_code& ec, std::size_t bytesReceived)
    {
      if(!ec)
      {
        if(bytesReceived == sizeof(Message::Header))
        {
          Message message(*reinterpret_cast<Message::Header*>(m_udpBuffer.data()));

          if(!m_localhostOnly || m_remoteEndpoint.address().is_loopback())
          {
            if(message.dataSize() == 0)
            {
              std::unique_ptr<Message> response = processMessage(message);
              if(response)
              {
                m_socketUDP.async_send_to(boost::asio::buffer(**response, response->size()), m_remoteEndpoint,
                  [this](const boost::system::error_code& /*ec*/, std::size_t /*bytesTransferred*/)
                  {
                    doReceive();
                  });
                return;
              }
            }
          }
        }
        doReceive();
      }
      else
        Log::log(id, LogMessage::E1003_UDP_RECEIVE_ERROR_X, ec.message());
    });
}

std::unique_ptr<Message> Server::processMessage(const Message& message)
{
  if(message.command() == Message::Command::Discover && message.isRequest())
  {
    std::unique_ptr<Message> response = Message::newResponse(message.command(), message.requestId());
    response->write(boost::asio::ip::host_name());
    response->write<uint16_t>(TRAINTASTIC_VERSION_MAJOR);
    response->write<uint16_t>(TRAINTASTIC_VERSION_MINOR);
    response->write<uint16_t>(TRAINTASTIC_VERSION_PATCH);
    assert(response->size() <= 1500); // must fit in a UDP packet
    return response;
  }

  return {};
}

void Server::doAccept()
{
  assert(IS_SERVER_THREAD);

  m_acceptor.async_accept(
    [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket)
    {
      if(!ec)
      {
        std::make_shared<HTTPConnection>(shared_from_this(), std::move(socket))->start();

        doAccept();
      }
      else
      {
        Log::log(id, LogMessage::E1004_TCP_ACCEPT_ERROR_X, ec.message());
      }
    });
}

http::message_generator Server::handleHTTPRequest(http::request<http::string_body>&& request)
{
  const auto target = request.target();
  if(target == "/")
  {
    return textHtml(request,
      "<!DOCTYPE html>"
      "<html>"
      "<head>"
        "<meta charset=\"utf-8\">"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, shrink-to-fit=no\">"
        "<title>Traintastic v" TRAINTASTIC_VERSION_FULL "</title>"
      "</head>"
      "<body>"
        "<h1>Traintastic <small>v" TRAINTASTIC_VERSION_FULL "</small></h1>"
        "<ul>"
          "<li><a href=\"/throttle\">Web throttle</a></li>"
        "</ul>"
      "</body>"
      "</html>");
  }
  if(target == "/favicon.ico")
  {
    return binary(request, contentTypeImageXIcon, Resource::shared::gfx::appicon_ico);
  }
  if(request.target() == "/css/normalize.css")
  {
    return textCss(request, Resource::www::css::normalize_css);
  }
  if(request.target() == "/css/throttle.css")
  {
#ifdef SERVE_FROM_FS
    const auto css = readFile(www / "css" / "throttle.css");
    return css ? textCss(request, *css) : notFound(request);
#else
    return textCss(request, Resource::www::css::throttle_css);
#endif
  }
  if(request.target() == "/js/throttle.js")
  {
#ifdef SERVE_FROM_FS
    const auto js = readFile(www / "js" / "throttle.js");
    return js ? textJavaScript(request, *js) : notFound(request);
#else
    return textJavaScript(request, Resource::www::js::throttle_js);
#endif
  }
  if(request.target() == "/throttle")
  {
#ifdef SERVE_FROM_FS
    const auto html = readFile(www / "throttle.html");
    return html ? textHtml(request, *html) : notFound(request);
#else
    return textHtml(request, Resource::www::throttle_html);
#endif
  }
  if(target == "/version")
  {
    return textPlain(request, TRAINTASTIC_VERSION_FULL);
  }
  return notFound(request);
}

bool Server::handleWebSocketUpgradeRequest(http::request<http::string_body>&& request, beast::tcp_stream& stream)
{
  if(request.target() == "/client")
  {
    return acceptWebSocketUpgradeRequest<ClientConnection>(std::move(request), stream);
  }
  if(request.target() == "/throttle")
  {
    return acceptWebSocketUpgradeRequest<WebThrottleConnection>(std::move(request), stream);
  }
  return false;
}

template<class T>
bool Server::acceptWebSocketUpgradeRequest(http::request<http::string_body>&& request, beast::tcp_stream& stream)
{
  namespace websocket = beast::websocket;

  beast::get_lowest_layer(stream).expires_never(); // disable HTTP timeout

  auto ws = std::make_shared<websocket::stream<beast::tcp_stream>>(std::move(stream));
  ws->set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));
  ws->set_option(websocket::stream_base::decorator(
    [](websocket::response_type& response)
    {
      response.set(beast::http::field::server, serverHeader);
    }));

  ws->async_accept(request,
    [this, ws](beast::error_code ec)
    {
      if(!ec)
      {
        auto connection = std::make_shared<T>(*this, ws);
        connection->start();

        EventLoop::call(
          [this, connection]()
          {
            Log::log(connection->id, LogMessage::I1003_NEW_CONNECTION);
            m_connections.push_back(connection);
          });
      }
      else
      {
        Log::log(id, LogMessage::E1004_TCP_ACCEPT_ERROR_X, ec.message());
      }
    });

  return true;
}
