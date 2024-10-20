#define ASIO_STANDALONE

// main.cpp
#include "crow.h"
#include "trading_system.hpp"
// #include "orderbook_manager.hpp"

#include <iostream>
#include <unordered_map>
// #include <mutex>
#include "deribit_ws_client.hpp" 



int main() {
    crow::SimpleApp app;  // Create the Crow app instance
    app.loglevel(crow::LogLevel::Debug);
    
    // Initialize the trading system
    TradingSystem trading("https://test.deribit.com");

    std::mutex mtx;;
    std::unordered_set<crow::websocket::connection*> users;

    CROW_ROUTE(app, "/ws")
        .websocket()
        .onopen([&](crow::websocket::connection& conn){
            CROW_LOG_INFO << "New WebSocket connection";
            std::lock_guard<std::mutex> lock(orderBookManager.mtx);
            std::string clientId = /* Get client ID from the connection or message */;
            users.insert(&conn);
            orderBookManager.registerConnection(clientId, conn); // Register connection with its client ID
        })
        .onclose([&](crow::websocket::connection& conn, const std::string& reason){
            CROW_LOG_INFO << "WebSocket connection closed: " << reason;
            std::lock_guard<std::mutex> lock(orderBookManager.mtx);
            users.erase(&conn);
            // Optionally unregister connection
        })
        .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary){
            // Handle incoming messages from clients
            // Parse action (subscribe/unsubscribe) and call respective methods
            // Example:
            // orderBookManager.subscribe(clientId, symbol);
        });

    std::thread deribitThread(listenToDeribit);

    // Define a route for placing an order
    CROW_ROUTE(app, "/orders").methods("POST"_method)
    ([&trading](const crow::request& req) {
        auto json_data = crow::json::load(req.body);
        if (!json_data) return crow::response(400, "Invalid JSON");

        std::unordered_map<std::string, std::string> params;
        params["instrument_name"] = json_data["instrument_name"].s();
        params["amount"] = std::to_string(static_cast<float>(json_data["amount"].d()));
        params["price"] = std::to_string(static_cast<unsigned int>(json_data["price"].i()));
        params["label"] = json_data["label"].s();
        params["type"] = json_data["type"].s();
        std::string action = json_data["action"].s();

        json response;
        if (action == "buy") {
            response = trading.buy(params); // Pass the map instead of individual params
        } else if (action == "sell") {
            response = trading.sell(params); // Pass the map instead of individual params
        } else {
            return crow::response(400, "Invalid action, use 'buy' or 'sell'");
        }

        return crow::response(200, "application/json", response.dump());
    });

    // Define a route for modifying an order
    CROW_ROUTE(app, "/orders/<string>").methods("PATCH"_method)
    ([&trading](const crow::request& req, const std::string& orderId) {
        auto json_data = crow::json::load(req.body);
        if (!json_data) return crow::response(400, "Invalid JSON");

        std::unordered_map<std::string, std::string> params;
        params["amount"] = std::to_string(static_cast<float>(json_data["amount"].d()));
        params["price"] = std::to_string(static_cast<unsigned int>(json_data["price"].i()));
        params["order_id"] = orderId;

        auto response = trading.modifyOrder(params);
        return crow::response(200, "application/json", response.dump());
    });

    // Define a route for canceling an order
    CROW_ROUTE(app, "/orders/<string>").methods("DELETE"_method)
    ([&trading](const crow::request& req, const std::string& orderId) {
        std::unordered_map<std::string, std::string> params;
        params["order_id"] = orderId;
       
        auto response = trading.cancelOrder(params);
        return crow::response(200, "application/json", response.dump());
    });

    // Define a route for getting the order book
    CROW_ROUTE(app, "/orders/orderbook").methods("GET"_method)
    ([&trading](const crow::request& req) {
        auto instrument_name = req.url_params.get("instrument_name");

        if (!instrument_name) {
            return crow::response(400, "Missing instrument_name query parameter");
        }

        std::unordered_map<std::string, std::string> params;
        params["instrument_name"] = std::string(instrument_name);
        
        auto response = trading.getOrderbook(params);
        return crow::response(200, "application/json", response.dump());
    });

    // Define a route for viewing current positions
    CROW_ROUTE(app, "/orders/positions").methods("GET"_method)
    ([&trading](const crow::request& req) {
        auto currency = req.url_params.get("currency");
        auto kind = req.url_params.get("kind");

        std::unordered_map<std::string, std::string> params;
        if (currency) {
            params["currency"] = std::string(currency);
        }
        if (kind) {
            params["kind"] = std::string(kind);
        }

        auto response = trading.viewCurrentPositions(params);
        return crow::response(200, "application/json", response.dump());
    });

    // Define a simple health check route
    CROW_ROUTE(app, "/") 
    ([&trading](const crow::request& req) {
        json ex1 = json::parse(R"(
        {
            "test": true
        }
        )");   
        return crow::response(200, ex1.dump());
    });

    std::cout << "Starting server..." << std::endl;
    
    // Start the server
    app.port(8080).multithreaded().run();
}
