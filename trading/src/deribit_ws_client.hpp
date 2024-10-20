#define ASIO_STANDALONE

#include <iostream>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <asio.hpp>
#include <asio/ssl.hpp>
#include <nlohmann/json.hpp>
#include <cstdlib> // for getenv

typedef websocketpp::client<websocketpp::config::asio_tls_client> ws_client;
typedef std::shared_ptr<asio::ssl::context> context_ptr;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;
using json = nlohmann::json;






using json = nlohmann::json;
 

static context_ptr on_tls_init() {
    context_ptr ctx = std::make_shared<asio::ssl::context>(asio::ssl::context::sslv23);

    try {
        ctx->set_options(asio::ssl::context::default_workarounds |
                         asio::ssl::context::no_sslv2 |
                         asio::ssl::context::no_sslv3 |
                         asio::ssl::context::single_dh_use);
    } catch (std::exception &e) {
        std::cout << "Error in context pointer: " << e.what() << std::endl;
    }
    return ctx;
}
void listenToDeribit(std::string asset) {
    if (!asset.empty()) {
        return;
    }
    ws_client client;
    
    // Initialize the ASIO transport
    client.init_asio();
    client.set_tls_init_handler(bind(&on_tls_init));

    std::string uri = "wss://test.deribit.com/ws/api/v2";
    
    // Define a handler for when the WebSocket connection is opened
    client.set_open_handler([&client](websocketpp::connection_hdl hdl) {
        std::cout << "Connected to Deribit WebSocket!" << std::endl;
        std::string assetName = "deribit_price_index" + asset;

        // Subscribe to the price index updates (no authentication needed)
        json subscribeMessage;
        subscribeMessage["jsonrpc"] = "2.0";
        subscribeMessage["id"] = 3600;
        subscribeMessage["method"] = "public/subscribe";
        subscribeMessage["params"] = {
            {"channels", {assetName}}
        };

        // Send subscription message
        client.send(hdl, subscribeMessage.dump(), websocketpp::frame::opcode::text);
    });

    // Handle incoming messages
    client.set_message_handler([&client](websocketpp::connection_hdl hdl, message_ptr msg) {
        std::cout << "Message received: " <<   msg->get_payload() << std::endl;
        // Process the received message if needed
    });

    client.set_close_handler([](websocketpp::connection_hdl hdl) {
        std::cout << "Connection closed to Deribit WebSocket!" << std::endl;
    });

    // Create a connection to the Deribit WebSocket API
    websocketpp::lib::error_code ec;
    ws_client::connection_ptr con = client.get_connection(uri, ec);
    if (ec) {
        std::cout << "Error during connection: " << ec.message() << std::endl;
        return;
    }

    // Connect and start the event loop
    client.connect(con);
    client.run();
}