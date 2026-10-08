#include "net/socket.h"

#include <iostream>
#include <exception>
#include <string>

int main() {
    try {
        net::WinsockRuntime runtime;
        auto server = net::Socket::connect_to("127.0.0.1", "18080");

        std::cout << "Da ket noi toi server\n";

        const std::string message = "Xin chao server";
        server.send_all(message);
        server.shutdown_send();

        const std::string response = net::receive_until_end(server);
        std::cout << "Server tra loi: " << response << '\n';

        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Loi client: " << error.what() << '\n';
        return 1;
    }
}