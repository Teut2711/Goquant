// trading_system.cpp
#include <iostream>

#include "trading_system.hpp"


TradingSystem::TradingSystem(const std::string & host): host_(host), client_(host.c_str()) {}


#include <sstream>
#include <string>
#include <unordered_map>
#include <optional>

std::string buildQueryString(const std::unordered_map<std::string, std::string>& params) {
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

// Place Order
json TradingSystem::buy(const std::string& instrument_name, float amount,unsigned int price, const std::string& label, const std::string& type)
 {
   std::string endpoint = "/api/v2/private/buy?amount=" + std::to_string(amount) +
    "&instrument_name=" + instrument_name +
    "&label=" + label +
    "&type=" + type +
    "&price=" + std::to_string(price);
;

   json response = sendGetRequest(endpoint);
  return response;
}

// Implementation of sell (assuming the sell endpoint follows a similar pattern)
json TradingSystem::sell(const std::string& instrument_name, float amount, unsigned int price, const std::string& label, const std::string& type){
   std::string endpoint = "/api/v2/private/sell?amount=" + std::to_string(amount) +
    "&instrument_name=" + instrument_name +
    "&label=" + label +
    "&type=" + type + 
    "&price=" + std::to_string(price);

  json response = sendGetRequest(endpoint);
  return response;
}

// Cancel Order
json TradingSystem::cancelOrder(const std::string & orderId) {
   std::string endpoint = "/api/v2/private/cancel?order_id=" + orderId;

  json response = sendGetRequest(endpoint);

   std::cout << "Cancel Order Response: " << response.dump(4) << std::endl;

   return response;
}

// Modify Order
json TradingSystem::modifyOrder(const std::string & orderId, unsigned int newPrice, float newamount) {
   std::string endpoint = "/api/v2/private/edit?advanced=implv&amount=" + std::to_string(newamount) +
    "&order_id=" + orderId + "&price=" + std::to_string(newPrice);

   json response = sendGetRequest(endpoint);

   std::cout << "Modify Order Response: " << response.dump(4) << std::endl;

  return response;
}

// Get Orderbook
json TradingSystem::getOrderbook(const std::string & instrument_name) {
  std::string endpoint = "/api/v2/public/get_order_book?instrument_name=" + instrument_name;
  json response = sendGetRequest(endpoint);
  std::cout << "Orderbook: " << response.dump(4) << std::endl;
  return response;
}

json TradingSystem::viewCurrentPositions(const std::string & currency = nullptr,
  const std::string & kind = nullptr) {
   
  std::string endpoint = "/api/v2/private/get_positions?currency=" + currency + "&kind=" + kind;

   json response = sendGetRequest(endpoint);

   std::cout << "Get Positions Response: " << response.dump(4) << std::endl;

   return response;
}

// Send a POST request with a JSON payload and Bearer token
json TradingSystem::sendPostRequest(const std::string & endpoint,
  const json & payload) {
   httplib::Headers headers = {
    {
      "Authorization",
      "Bearer " + authData.authToken
    }, 
    {
      "Content-Type",
      "application/json"
    } 
  };
  const std::string url = host_ + endpoint;

  // First attempt to send the request
  auto res = client_.Post(url.c_str(), headers, payload.dump(), "application/json");

  if (res && res -> status == 200) {
    return json::parse(res -> body);
  } else if (res && res -> status == 400) {
    // Log the unauthorized error and refresh token
    std::cerr << "Unauthorized (401): POST request to " << endpoint << ". Refreshing token." << std::endl;

    // Refresh auth token and retry
    refreshAuthToken();

    // Retry the request after refreshing token
    headers = {
      {
        "Authorization",
        "Bearer " + authData.authToken
      }, // Add Bearer token to headers
      {
        "Content-Type",
        "application/json"
      } // Ensure Content-Type is JSON
    };
    res = client_.Post(url.c_str(), headers, payload.dump(), "application/json");
    if (res && res -> status == 200) {
      return json::parse(res -> body);
    } else {
      std::cerr << "Error: Failed to send POST request again to " << url << " after refreshing token" << std::endl;
     
    }
  } else {
    // Log any other errors
    std::cerr << "Error: Failed to send POST request to " << url << " (status: " << (res ? res -> status : 0) << ")" << std::endl;
  }

  return json::parse(res -> body);
 // Return empty JSON on failure
}

// Send a GET request with Bearer token
json TradingSystem::sendGetRequest(const std::string & endpoint) {
  // Set headers with Authorization: Bearer <token>
   
    httplib::Headers headers;

    // Check if the endpoint contains "private" to set the authorization header
    if (endpoint.find("private") != std::string::npos) {
        headers = {
            { "Authorization", "Bearer " + authData.authToken } // Add Bearer token to headers
        };
    }
  const std::string url = host_ + endpoint;

  // First attempt to send the request
  auto res = client_.Get(url.c_str(), headers);

  if (res && res -> status == 200) {
    return json::parse(res -> body);
  } else if (res && res -> status == 400) {
    // Log the unauthorized error and refresh token
    std::cerr << "Unauthorized (401): GET request to " << endpoint << ". Refreshing token." << std::endl;

    // Refresh auth token and retry
    refreshAuthToken();

    // Retry the request after refreshing token
    headers = {
      {
        "Authorization",
        "Bearer " + authData.authToken
      }, // Add Bearer token to headers
      {
        "Content-Type",
        "application/json"
      } // Ensure Content-Type is JSON
    };
    res = client_.Get(url.c_str(), headers);
    if (res && res -> status == 200) {
      return json::parse(res -> body);
    } else {
      std::cerr << "Error: Failed to send GET request again to " << url << " after refreshing token" << std::endl;
    }
  } else {
    // Log any other errors
    std::cerr << "Error: Failed to send GET request to " << url << " (status: " << (res ? res -> status : 0) << ")" << std::endl;
  }

  return json::parse(res -> body);  // Return empty JSON on failure
}

void TradingSystem::refreshAuthToken() {
    const char* client_id = std::getenv("CLIENT_ID");
    const char* client_secret = std::getenv("CLIENT_SECRET");

    if (!client_id || !client_secret) {
        throw std::runtime_error("Environment variables for client_id and/or client_secret are not set.");
    }

    // Create the endpoint with query parameters
    std::string endpoint = "/api/v2/public/auth?client_id=" + std::string(client_id) +
                            "&client_secret=" + std::string(client_secret) +
                            "&grant_type=client_credentials";

    // Send a GET request to the constructed URL
    auto res = sendGetRequest(endpoint);
    
    std::cout << "Response auth: " << res.dump() << std::endl;  // Log the response
    if (!res.empty()) {
        authData.authToken = res["result"]["access_token"].get<std::string>();
        long expires_in = res["result"]["expires_in"].get<long>(); // millisec
        authData.expiration = std::chrono::milliseconds(expires_in + 60000);
        std::cout << "Access token: " << authData.authToken << std::endl;
    } else {
        std::cerr << "Failed to refresh auth token." << std::endl;
    }
}

// // Send a POST request with a JSON payload
// json TradingSystem::sendPostRequest(const std::string& endpoint, const json& payload) {
//     // Retry the request with authentication if needed
//     return retryWithAuth([this](const std::string& endpoint, const json& payload) {
//         auto res = client_.Post((host_ + endpoint).c_str(), payload.dump(), "application/json");
//         if (res && res->status == 200) {
//             return json::parse(res->body);
//         } else {
//             std::cerr << "Error: Failed to send POST request to " << endpoint << std::endl;

//             return json{}; // Return empty JSON on failure
//         }
//     }, endpoint, payload);
// }

// // Send a GET request
// json TradingSystem::sendGetRequest(const std::string& endpoint) {
//     // Retry the request with authentication if needed
//     return retryWithAuth([this](const std::string& endpoint) {
//         auto res = client_.Get((host_ + endpoint).c_str());
//         if (res && res->status == 200) {
//             std::cout << res->body;
//             return json::parse(res->body);
//         } else {
//             std::cerr << "Error: Failed to send GET request to " << endpoint << std::endl;
//             return json{}; // Return empty JSON on failure
//         }
//     }, endpoint);
// }

// template<typename Func, typename... Args>
// json TradingSystem::retryWithAuth(Func func, Args&&... args) {
//     auto result = func(std::forward<Args>(args)...);
//     if (result.is_null() || (result.contains("error") && result["error"].template get<std::string>() == "401")) {
//         refreshAuthToken();  
//         result = func(std::forward<Args>(args)...);  
//     }
//     return result;
// }