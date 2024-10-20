#include <nlohmann/json.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include "websocket_client.hpp"
#include "http_client.hpp"


using json = nlohmann::json;

 namespace net = boost::asio;            // from <boost/asio.hpp>

int main() {
         
        // Host and port for Deribit test WebSocket API
           auto const host = "test.deribit.com";
           auto const port = "443";
           auto const text ="Hello, world!";
          [[maybe_unused]] auto const apiKey = getenv("API_KEY");
          
          net::io_context ioc;
          std::make_shared<local_Http::session>(ioc)->run(host, port, target, version);
          std::make_shared<local_Websocket::session>(ioc)->run(host, port, text);
          ioc.run();
   
    return EXIT_SUCCESS;
    
    }
