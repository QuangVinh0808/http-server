#include "net/socket.h"
#include "http/http_parser.h"

#include <string>
#include <exception>
#include <iostream>

int main() {
    try {
        net::WinsockRuntime runtime;
        net::Socket server = net::Socket::listen_on("127.0.0.1", "18080");

        std::string note;
        bool has_note = false;

        while (true) {
            net::Socket client = server.accept_client();
            try {
                HttpRequest request;
                int status = read_http_request(client, request);
                std::string body;

                if (status != 200) {
                    body = "Http request bi tu choi !\n";
                } else {
                    std::string path = request.target.substr(0, request.target.find('?'));

                    if (path != "/note") {
                        status = 404;
                        body = "404 not found\n";
                    } else if (request.method == "GET" || request.method == "HEAD") {
                        if (has_note) {
                            body = note;
                        } else {
                            status = 404;
                            body = "Note khong ton tai !\n";
                        }
                    } else if (request.method == "POST") {
                        if (has_note) {
                            status = 409;
                            body = "Note da ton tai !\n";
                        } else {
                            note = request.body;
                            has_note = true;
                            status = 201;
                            body = "Da post note thanh cong !\n";
                        }
                    } else if (request.method == "PUT") {
                        note = request.body;
                        status = has_note ? 200 : 201;
                        body = has_note ? "Da ghi de\n" : "Da tao moi\n";
                        has_note = true;
                    } else if (request.method == "DELETE") {
                        if (has_note) {
                            note.clear();
                            has_note = false;
                            body = "Da xoa het note !\n";
                        } else {
                            status = 404;
                            body = "Khong ton tai note nao !\n";
                        }
                    } else if (request.method == "OPTIONS" ||
                               request.method == "PATCH" ||
                               request.method == "TRACE" ||
                               request.method == "CONNECT") {
                        status = 405;
                        body = "Method khong duoc ho tro\n";
                    } else {
                        status = 501;
                        body = "Method chua duoc trien khai !\n";
                    }
                }

                const std::string response = make_http_response(
                    status, body, request.method == "HEAD"
                );
                client.send_all(response);
                client.shutdown_send();

                std::cout << request.method << " " << request.target
                          << " -> " << status << "\n";
            } catch (const std::exception& error) {
                std::cerr << "Loi client: " << error.what() << "\n";
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "Loi server: " << error.what() << "\n";
        return 1;
    }
}