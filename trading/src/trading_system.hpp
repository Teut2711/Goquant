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

class TradingSystem {
public:
    TradingSystem(const std::string& host);
    
  json buy(const std::string& instrument_name, float amount,unsigned int price, const std::string& label, const std::string& type);
json sell(const std::string& instrument_name, float amount, unsigned int price, const std::string& label, const std::string& type);

 json cancelOrder(const std::string& orderId);
json modifyOrder(const std::string& orderId, unsigned int newPrice, float newAmount);
    json getOrderbook(const std::string& instrument_name);
    json viewCurrentPositions(const std::string& currency, const std::string& kind) ;


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
