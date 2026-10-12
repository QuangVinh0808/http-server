#include "net/socket.h"

#include <exception>
#include <iostream>
#include <string>

int main() {
    try {
        net::WinsockRuntime runtime;
        net::Socket client = net::Socket::connect_to("127.0.0.1", "18080");

        const std::string request =
            "GET /note HTTP/1.1\r\n"
            "Host: 127.0.0.1:18080\r\n"
            "Connection: close\r\n"
            "\r\n";

        client.send_all(request);
        const std::string response = net::receive_until_end(client);
        std::cout << response;
    } catch (const std::exception& error) {
        std::cerr << "Loi client: " << error.what() << "\n";
        return 1;
    }
}