#include <nlohmann/json.hpp>

#include <boost/beast/core.hpp>

#include <boost/beast/http.hpp>

#include <boost/beast/version.hpp>

#include <boost/asio/strand.hpp>

#include <cstdlib>

#include <functional>

#include <iostream>

#include <memory>

#include <string>
#include "http_client.hpp"



using json = nlohmann::json;

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http; // from <boost/beast/http.hpp>
namespace net = boost::asio; // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

using namespace local_Http;



 std::string getOauthToken(net::io_context& ioc, const std::string& host, const std::string& target) {
    try {
        // Create a TCP socket and resolver
        tcp::resolver resolver(ioc);
        beast::tcp_stream stream(ioc);

        // Look up the domain name
        auto const results = resolver.resolve(host, "443");

        // Make the connection on the IP address we get from a lookup
        stream.connect(results);

        // Set up an HTTP GET request message
        http::request<http::string_body> req{http::verb::get, target, 11};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(http::field::content_type, "application/json");

        // Send the HTTP request to the remote host
        http::write(stream, req);

        // Declare a container to hold the response
        beast::flat_buffer buffer;
        http::response<http::string_body> res;

        // Receive the HTTP response
        http::read(stream, buffer, res);

        // Parse JSON response to extract access_token
        auto json_response = json::parse(res.body());
        std::string access_token = json_response["result"]["access_token"];

        // Gracefully close the socket
        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);

        if (ec && ec != beast::errc::not_connected) {
            throw beast::system_error{ec};
        }

        return access_token;

    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return "";
    }
}



void
fail(beast::error_code ec, char
  const * what) {
  std::cerr << what << ": " << ec.message() << "\n";
}
session::session(net::io_context & ioc): resolver_(net::make_strand(ioc)), stream_(net::make_strand(ioc)) {}

// Start the asynchronous operation
void
session::run(
  char
  const * host,
  char
  const * port,
  char
  const * target,
  int version) {
  // Set up an HTTP GET request message
  req_.version(version);
  req_.method(http::verb::get);
  req_.target(target);
  req_.set(http::field::host, host);
  req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

  // Look up the domain name
  resolver_.async_resolve(
    host,
    port,
    beast::bind_front_handler( &
      session::on_resolve,
      shared_from_this()));
}

void
session::on_resolve(
  beast::error_code ec,
  tcp::resolver::results_type results) {
  if (ec)
    return fail(ec, "resolve");

  // Set a timeout on the operation
  stream_.expires_after(std::chrono::seconds(30));

  // Make the connection on the IP address we get from a lookup
  stream_.async_connect(
    results,
    beast::bind_front_handler( &
      session::on_connect,
      shared_from_this()));
}

void
session::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type) {
  if (ec)
    return fail(ec, "connect");

  // Set a timeout on the operation
  stream_.expires_after(std::chrono::seconds(30));

  // Send the HTTP request to the remote host
  http::async_write(stream_, req_,
    beast::bind_front_handler( &
      session::on_write,
      shared_from_this()));
}

void
session::on_write(
  beast::error_code ec,
  std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);

  if (ec)
    return fail(ec, "write");

  // Receive the HTTP response
  http::async_read(stream_, buffer_, res_,
    beast::bind_front_handler( &
      session::on_read,
      shared_from_this()));
}

void
session::on_read(
  beast::error_code ec,
  std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);

  if (ec)
    return fail(ec, "read");

  // Write the message to standard out
  std::cout << res_ << std::endl;

  // Gracefully close the socket
  stream_.socket().shutdown(tcp::socket::shutdown_both, ec);

  // not_connected happens sometimes so don't bother reporting it.
  if (ec && ec != beast::errc::not_connected)
    return fail(ec, "shutdown");
  }
  // If we get here then the connection is closed gracefully

 