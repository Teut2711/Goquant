// trading_system.hpp
#ifndef TRADING_SYSTEM_HPP
#define TRADING_SYSTEM_HPP
#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <string>
#include <chrono>
#include <functional>
#include <nlohmann/json.hpp>
#include "httplib.h"

using json = nlohmann::json;

struct AuthData {
    std::string authToken;
    std::chrono::milliseconds expiration;
};


 
inline std::string buildQueryString(const std::unordered_map<std::string, std::string>& params) {
    std::ostringstream query;
    bool first = true;

    for (const auto& [key, value] : params) {
        if (first) {
            first = false; 
        } else {
            query << "&"; 
        }
        query << key << "=" << value;
    }
    return query.str();
}


class TradingSystem {
public:
    TradingSystem(const std::string& host);
    
  json buy(const std::unordered_map<std::string, std::string>& params);
json sell(const std::unordered_map<std::string, std::string>& params);

 json cancelOrder(const std::unordered_map<std::string, std::string>& params);
json modifyOrder(const std::unordered_map<std::string, std::string>& params);
    json getOrderbook(const std::unordered_map<std::string, std::string>& params);
    json viewCurrentPositions(const std::unordered_map<std::string, std::string>& params) ;


private:
    std::string host_;
    httplib::Client client_;
    AuthData authData;

    json sendPostRequest(const std::string& endpoint, const json& payload);
    json sendGetRequest(const std::string& endpoint);
    // Decorator function
    // template<typename Func, typename... Args>
    // json retryWithAuth(Func func, Args&&... args);

    void refreshAuthToken();
};

#endif  // TRADING_SYSTEM_HPP
