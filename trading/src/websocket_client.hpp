#pragma once

#include <nlohmann/json.hpp>

#include <boost/beast/core.hpp>

#include <boost/beast/websocket.hpp>

#include <boost/asio/strand.hpp>

#include <cstdlib>

#include <functional>

#include <iostream>

#include <memory>

#include <string>
using json = nlohmann::json;

  namespace beast = boost::beast;
  namespace http = beast::http;
  namespace websocket = beast::websocket;
  namespace net = boost::asio;
  using tcp = boost::asio::ip::tcp;


  void
  fail(beast::error_code ec, char
    const * what);
namespace local_Websocket
{

  

  class session: public std::enable_shared_from_this < session > {
    tcp::resolver resolver_;
    websocket::stream < beast::tcp_stream > ws_;
    beast::flat_buffer buffer_;
    std::string host_;
    std::string text_;

    public:
      // Resolver and socket require an io_context
      explicit
    session(net::io_context & ioc);

    // Start the asynchronous operation
    void
    run(
      char
      const * host,
      char
      const * port,
      char
      const * text);

    void
    on_resolve(
      beast::error_code ec,
      tcp::resolver::results_type results);

    void
    on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep);

    void
    on_handshake(beast::error_code ec);

    void
    on_write(
      beast::error_code ec,
      std::size_t bytes_transferred);

    void
    on_read(
      beast::error_code ec,
      std::size_t bytes_transferred);
    void
    on_close(beast::error_code ec);

  }; // namespace local_Websocket;
}