#include "orderbook_manager.hpp"

void OrderBookManager::subscribe(const std::string& clientId, const std::string& symbol) {
    std::lock_guard<std::mutex> lock(mtx);
    subscriptions[clientId].insert(symbol);
}

void OrderBookManager::unsubscribe(const std::string& clientId, const std::string& symbol) {
    std::lock_guard<std::mutex> lock(mtx);
    auto& symbols = subscriptions[clientId];
    symbols.erase(symbol);
}

void OrderBookManager::notifySubscribers(const std::string& symbol, const std::string& message) {
    std::lock_guard<std::mutex> lock(mtx);
    for (const auto& [clientId, symbols] : subscriptions) {
        if (symbols.find(symbol) != symbols.end()) {
            auto conn = connections[clientId];
            if (conn) {
                conn->send_text(message);  
            }
        }
    }
}

void OrderBookManager::registerConnection(const std::string& clientId, crow::websocket::connection& conn) {
    std::lock_guard<std::mutex> lock(mtx);
    connections[clientId] = &conn;
}

void OrderBookManager::onDeribitMessage(const std::string& message) {
    // Parse the message to extract the symbol and then notify subscribers
    // Example: Assuming the message format is known and it contains the symbol
    // Extract the symbol from the message
    // Notify the subscribers for the symbol
    std::string symbol = /* extract from message */;
    notifySubscribers(symbol, message);
}
