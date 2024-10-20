// main.cpp
#include "crow.h"
#include "trading_system.hpp"
#include <iostream>
 
int main() {
    crow::SimpleApp app;  // Create the Crow app instance
    app.loglevel(crow::LogLevel::Debug);
    // Initialize the trading system
    TradingSystem trading("https://test.deribit.com");

// Define a route for placing an order
CROW_ROUTE(app, "/orders") .methods("POST"_method)
([&trading](const crow::request& req) {
    auto json_data = crow::json::load(req.body);
    std::string instrument_name = json_data["instrument_name"].s();
    float amount = static_cast<float>(json_data["amount"].d()); // `.d()` for extracting float/double
    unsigned int price = static_cast<unsigned int>(json_data["price"].i()); // Use `.i()` for integer extraction
    std::string label = json_data["label"].s();
    std::string type = json_data["type"].s();
    std::string action = json_data["action"].s();

    json response;
    if (action == "buy") {
        response = trading.buy(instrument_name, amount,price,  label, type);
    } else if (action == "sell") {
        response = trading.sell(instrument_name, amount,price,  label, type);
    } else {
        return crow::response(400, "Invalid action, use 'buy' or 'sell'");
    }

    return crow::response(200,"application/json", response.dump());
});

// Define a route for modifying an order
CROW_ROUTE(app, "/orders/<string>") .methods("PATCH"_method)
([&trading](const crow::request& req, const std::string& orderId) {
    auto json_data = crow::json::load(req.body);
    float newAmount = static_cast<float>(json_data["amount"].d()); // `.d()` for extracting float/double
    unsigned int newPrice = static_cast<unsigned int>(json_data["price"].i()); // Use `.i()` for integer extraction
   
    auto response = trading.modifyOrder(orderId, newPrice, newAmount);
    return crow::response(200,"application/json", response.dump());
});

// Define a route for canceling an order
CROW_ROUTE(app, "/orders/<string>") .methods("DELETE"_method)
([&trading](const crow::request& req, const std::string& orderId) {
    auto response = trading.cancelOrder(orderId);
    return crow::response(200,"application/json", response.dump());
});


CROW_ROUTE(app, "/orders/orderbook").methods("GET"_method)
([&trading](const crow::request& req) {
    // Get the instrument_name from query parameters
    auto instrument_name = req.url_params.get("instrument_name");

    // Check if instrument_name is provided
    if (!instrument_name) {
        return crow::response(400, "Missing instrument_name query parameter");
    }

    // Call the trading method with the instrument_name
    auto response = trading.getOrderbook(std::string(instrument_name));
    return crow::response(200,"application/json", response.dump());
});

// Define a route for viewing current positions
// Define a route for viewing current positions
CROW_ROUTE(app, "/orders/positions").methods("GET"_method)
([&trading](const crow::request& req) {
    // Get the currency and kind from query parameters
    auto currency = req.url_params.get("currency");
    auto kind = req.url_params.get("kind");

    // Check if both currency and kind are provided
    if (!currency || !kind) {
        return crow::response(400, "Missing currency or kind query parameters");
    }

    // Call the trading method with currency and kind
    auto response = trading.viewCurrentPositions(std::string(currency), std::string(kind));
    return crow::response(200,"application/json", response.dump());

});

 // Define a route for viewing current positions
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
