#include "net/socket.h"

#include <iostream>
#include <exception>
#include <string>

int main() {
    try {
        net::WinsockRuntime runtime;
        auto server = net::Socket::listen_on("127.0.0.1", "18080");

        std::cout << "Server dang lang nghe tai 127.0.0.1:18080\n";

        for (;;) {
            auto client = server.accept_client();
            std::cout << "Client da ket noi\n";

            try {
                const std::string request = net::receive_until_end(client);
                std::cout << "Client gui: " << request << '\n';

                const std::string response = "Server da nhan: " + request;
                client.send_all(response);
                client.shutdown_send();
            } catch (const std::exception& error) {
                std::cerr << "Loi xu ly client: " << error.what() << '\n';
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "Loi server: " << error.what() << '\n';
        return 1;
    }
}