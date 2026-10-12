#include "net/socket.h"
#include "http/http_parser.h"

#include <exception>
#include <iostream>
#include <string>

int main() {
    constexpr int REQUEST_TIMEOUT_MS = 10000;
    constexpr DWORD SEND_TIMEOUT_MS = 5000;

    try {
        net::WinsockRuntime runtime;
        net::Socket server = net::Socket::listen_on("127.0.0.1", "18080");

        std::string note;
        bool has_note = false;

        std::cout << "Server: http://127.0.0.1:18080/note\n";

        while (true) {
            net::Socket client = server.accept_client();

            try {
                client.set_send_timeout(SEND_TIMEOUT_MS);

                HttpRequest request;
                int status = read_http_request(client, request, REQUEST_TIMEOUT_MS);
                std::string body;

                if (status == 408) {
                    body = "Qua thoi gian nhan request !\n";
                } else if (status != 200) {
                    body = "HTTP request bi tu choi !\n";
                } else {
                    const std::string path = request.target.substr(0, request.target.find('?'));

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
                        body = "Method chua duoc trien khai\n";
                    }
                }

                client.send_all(make_http_response( status, body, request.method == "HEAD"));
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