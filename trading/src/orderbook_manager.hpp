#ifndef ORDER_BOOK_MANAGER_HPP
#define ORDER_BOOK_MANAGER_HPP

#include "crow.h"
#include <unordered_map>
#include <unordered_set>
#include <mutex>

class OrderBookManager {
public:
    void subscribe(const std::string& clientId, const std::string& symbol);
    void unsubscribe(const std::string& clientId, const std::string& symbol);
    void notifySubscribers(const std::string& symbol, const std::string& message);
    void registerConnection(const std::string& clientId, crow::websocket::connection& conn);
    void onDeribitMessage(const std::string& message); // Call this on receiving a message from Deribit WebSocket

private:
    std::unordered_map<std::string, std::unordered_set<std::string>> subscriptions; // clientId -> symbols
    std::unordered_map<std::string, crow::websocket::connection*> connections; // clientId -> connection
    std::mutex mtx; // For thread-safe operations
};

#endif // ORDER_BOOK_MANAGER_HPP
