#pragma once
#define ASIO_STANDALONE
#include <iostream>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <asio.hpp>
#include <asio/ssl.hpp>
#include <nlohmann/json.hpp>
#include <cstdlib> // for getenv
#include<crow.h>
#include<unordered_set>
#include <mutex>


typedef websocketpp::client<websocketpp::config::asio_tls_client> ws_client;
typedef std::shared_ptr<asio::ssl::context> context_ptr;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;
using json = nlohmann::json;






using json = nlohmann::json;
std::mutex mtx;
std::unordered_map<std::string, std::unordered_set<crow::websocket::connection&>> usersData;

bool shouldUpdateSubscriptions = false;


 

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
void listenToDeribit( std::unordered_map<std::string, std::unordered_set<crow::websocket::connection&>>& users_data) {
    
    ws_client client;
    
    // Initialize the ASIO transport
    client.init_asio();
    client.set_tls_init_handler(bind(&on_tls_init));

    std::string uri = "wss://test.deribit.com/ws/api/v2";
    
    // Define a handler for when the WebSocket connection is opened
    client.set_open_handler([&client, &users_data](websocketpp::connection_hdl hdl) {
        std::cout << "Connected to Deribit WebSocket!" << std::endl;

        // Collect channels from users_data
        std::vector<std::string> channels;
        {
            std::lock_guard<std::mutex> lock(mtx); // Ensure thread-safe access to users_data
            for (const auto& entry : users_data) {
                std::string assetName = "deribit_price_index." + entry.first;
                channels.push_back(assetName);
            }
        }

        // Subscribe to the price index updates
        json subscribeMessage;
        subscribeMessage["jsonrpc"] = "2.0";
        subscribeMessage["id"] = 3600;
        subscribeMessage["method"] = "public/subscribe";
        subscribeMessage["params"] = {
            {"channels", channels}
        };

        // Send subscription message
        client.send(hdl, subscribeMessage.dump(), websocketpp::frame::opcode::text);
    });
    // Handle incoming messages
      client.set_message_handler([&client, &users_data](websocketpp::connection_hdl hdl, ws_client::message_ptr msg) {
        // Parse the incoming message
        json message = json::parse(msg->get_payload());

        // Extract the channel name (e.g., "deribit_price_index.BTC")
        std::string channel_name = message["params"]["channel"];

        // Remove the "deribit_price_index." prefix to get the actual asset name
        std::string asset_name = channel_name.substr(std::string("deribit_price_index.").length());

        // Forward the message to all users subscribed to this asset
        std::lock_guard<std::mutex> lock(mtx); // Ensure thread-safe access to users_data
        auto it = users_data.find(asset_name);
        if (it != users_data.end()) {
            for (const auto& user_conn : it->second) {
                 user_conn.send_text(msg->get_payload());
            }
        }

        std::cout << "Message received: " << msg->get_payload() << std::endl;
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



// This thread will listen to Deribit, constantly checking for updated topics
void subscriptionThread(std::unordered_map<std::string, std::unordered_set<crow::websocket::connection&>>& usersData, std::atomic<bool>& shouldUpdateSubscriptions) {
    while (true) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (shouldUpdateSubscriptions) {
                shouldUpdateSubscriptions = false;  // Reset flag
                listenToDeribit(usersData);   
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));  // Adjust the sleep interval as needed
    }
}