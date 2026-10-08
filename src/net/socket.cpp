#include "net/socket.h"

#include <stdexcept>
#include <algorithm>
#include <memory>
#include <utility>
#include <limits>

namespace net {

namespace {
    [[noreturn]] void fail(const char* operation, int code) {
        throw std::runtime_error(std::string("That bai ") + operation + " voi ma loi la: " + std::to_string(code));
    }

    using AddressList = std::unique_ptr<addrinfo, decltype(&freeaddrinfo)>;

    AddressList resolve(const char* host, const char* port, bool passive, int family, int socktype, int protocol) {
        addrinfo hints{};
        hints.ai_family = family;
        hints.ai_socktype = socktype;
        hints.ai_protocol = protocol;
        hints.ai_flags = passive ? AI_PASSIVE : 0;

        addrinfo* result = nullptr;
        int code = getaddrinfo(host, port, &hints, &result);
        if (code != 0) {
            fail("getaddrinfo", code);
        }

        return AddressList(result, &freeaddrinfo);
    }
}

WinsockRuntime::WinsockRuntime() {
    WSADATA wsa_data{};
    int code = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (code != 0) {
        fail("WSAStartup", code);
    }
}

WinsockRuntime::~WinsockRuntime() {
    WSACleanup();
}

Socket::Socket(SOCKET handle) noexcept : handle_(handle) {}

Socket::~Socket() {
    if (handle_ != INVALID_SOCKET) {
        closesocket(handle_);
    }
}

Socket::Socket(Socket&& other) noexcept : handle_(other.handle_) {
    other.handle_ = INVALID_SOCKET;
}

Socket& Socket::operator=(Socket&& other) noexcept {
    if (this != &other) {
        if (handle_ != INVALID_SOCKET) {
            closesocket(handle_);
        }
        handle_ = other.handle_;
        other.handle_ = INVALID_SOCKET;
    }
    return *this;
}

Socket Socket::listen_on(const char* host, const char* port) {
    auto addresses = resolve(host, port, true, AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int last_error = WSAEADDRNOTAVAIL;

    for (auto* address = addresses.get(); address != nullptr; address = address->ai_next) {
        const SOCKET handle = ::socket(address->ai_family, address->ai_socktype, address->ai_protocol);
        if (handle == INVALID_SOCKET) {
            last_error = WSAGetLastError();
            continue;
        }

        Socket candidate(handle);
        if (::bind(handle, address->ai_addr, static_cast<int>(address->ai_addrlen)) == SOCKET_ERROR) {
            last_error = WSAGetLastError();
            continue;
        }
        if (::listen(handle, SOMAXCONN) == SOCKET_ERROR) {
            last_error = WSAGetLastError();
            continue;
        }
        return candidate;
    }

    fail("socket/bind/listen", last_error);
}

Socket Socket::connect_to(const char* host, const char* port) {
    auto addresses = resolve(host, port, false, AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int last_error = WSAEHOSTUNREACH;

    for (auto* address = addresses.get(); address != nullptr; address = address->ai_next) {
        const SOCKET handle = ::socket(address->ai_family, address->ai_socktype, address->ai_protocol);
        if (handle == INVALID_SOCKET) {
            last_error = WSAGetLastError();
            continue;
        }

        Socket candidate(handle);
        if (::connect(handle, address->ai_addr, static_cast<int>(address->ai_addrlen)) == SOCKET_ERROR) {
            last_error = WSAGetLastError();
            continue;
        }
        return candidate;
    }

    fail("socket/connect", last_error);
}

Socket Socket::accept_client() const {
    const SOCKET client = ::accept(handle_, nullptr, nullptr);
    if (client == INVALID_SOCKET) {
        fail("accept", WSAGetLastError());
    }
    return Socket(client);
}

void Socket::send_all(std::string_view data) const {
    while (!data.empty()) {
        const int length = static_cast<int>(std::min(data.size(), static_cast<std::size_t>(std::numeric_limits<int>::max())));
        const int sent = ::send(handle_, data.data(), length, 0);
        if (sent == SOCKET_ERROR) {
            fail("send", WSAGetLastError());
        }
        if (sent == 0) {
            throw std::runtime_error("send made no progress");
        }
        data.remove_prefix(static_cast<std::size_t>(sent));
    }
}

std::string Socket::receive_chunk() const {
    char buffer[4096];
    const int received = ::recv(handle_, buffer, sizeof(buffer), 0);
    if (received == SOCKET_ERROR) {
        fail("recv", WSAGetLastError());
    }
    return std::string(buffer, static_cast<std::size_t>(received));
}

void Socket::shutdown_send() const {
    if (::shutdown(handle_, SD_SEND) == SOCKET_ERROR) {
        fail("shutdown", WSAGetLastError());
    }
}

std::string receive_until_end(const Socket& socket, std::size_t limit) {
    std::string data;
    for (;;) {
        const auto chunk = socket.receive_chunk();
        if (chunk.empty()) {
            return data;
        }
        if (chunk.size() > limit - data.size()) {
            throw std::runtime_error("Received data exceeds demo size limit");
        }
        data.append(chunk);
    }
}

} // namespace net