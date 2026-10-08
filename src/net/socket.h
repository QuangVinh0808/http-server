#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>

#include <string>
#include <cstddef>
#include <string_view>

namespace net {
class WinsockRuntime {
public:
    WinsockRuntime();
    ~WinsockRuntime();

    WinsockRuntime(const WinsockRuntime&) = delete;
    WinsockRuntime& operator=(const WinsockRuntime&) = delete;
};


class Socket {
public:
    ~Socket();

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    static Socket listen_on(const char* host, const char* port);
    static Socket connect_to(const char* host, const char* port);
    Socket accept_client() const;

    void send_all(std::string_view data) const;
    std::string receive_chunk() const;
    void shutdown_send() const;

private:
    explicit Socket(SOCKET socket) noexcept;
    SOCKET handle_ = INVALID_SOCKET;
};

std::string receive_until_end(const Socket& socket, std::size_t limit = 64 * 1024);

} // namespace net
