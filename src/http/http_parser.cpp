#include "http_parser.h"

#include <ctime>

static constexpr std::size_t MAX_HEADER_SIZE = 8192;
static constexpr std::size_t MAX_BODY_SIZE = 65536;

static std::string trim(const std::string& text) {
    const auto first = text.find_first_not_of(" \t");
    if (first == std::string::npos) {
        return "";
    }
    const auto last = text.find_last_not_of(" \t");
    return text.substr(first, last - first + 1);
}

static std::string lowercase(std::string text) {
    for (char& ch : text) {
        if (ch >= 'A' && ch <= 'Z') {
            ch = static_cast<char>(ch - 'A' + 'a');
        }
    }
    return text;
}

static bool is_token(const std::string& text) {
    if (text.empty()) {
        return false;
    }
    const std::string symbols = "!#$%&'*+-.^_`|~";
    for (unsigned char ch : text) {
        const bool letter = (ch >= 'A' && ch <= 'Z') ||
                            (ch >= 'a' && ch <= 'z');
        const bool digit = ch >= '0' && ch <= '9';
        if (!letter && !digit && symbols.find(ch) == std::string::npos) {
            return false;
        }
    }
    return true;
}

static bool parse_length(const std::string& text, std::size_t& length) {
    if (text.empty()) {
        return false;
    }
    length = 0;
    for (char ch : text) {
        if (ch < '0' || ch > '9') {
            return false;
        }
        const std::size_t digit = static_cast<std::size_t>(ch - '0');
        if (length > (MAX_BODY_SIZE - digit) / 10) {
            return false;
        }
        length = length * 10 + digit;
    }
    return true;
}

int read_http_request(const net::Socket& client, HttpRequest& request) {
    request = HttpRequest{};
    std::string data;
    std::size_t header_end;

    // Nhan den khi het headers.
    while (true) {
        header_end = data.find("\r\n\r\n");
        if (header_end != std::string::npos) {
            if (header_end + 4 > MAX_HEADER_SIZE) {
                return 431;
            }
            break;
        }
        if (data.size() >= MAX_HEADER_SIZE) {
            return 431;
        }
        const std::string chunk = client.receive_chunk();
        if (chunk.empty()) {
            return 400;
        }
        data += chunk;
    }

    // Doc dong dau: METHOD TARGET HTTP/VERSION.
    const std::size_t line_end = data.find("\r\n");
    const std::string first_line = data.substr(0, line_end);
    const std::size_t first_space = first_line.find(' ');
    if (first_space == std::string::npos) {
        return 400;
    }
    request.method = first_line.substr(0, first_space);
    const std::size_t second_space = first_line.find(' ', first_space + 1);
    if (second_space == std::string::npos) {
        return 400;
    }
    request.target = first_line.substr(
        first_space + 1,
        second_space - first_space - 1
    );
    request.version = first_line.substr(second_space + 1);

    if (!is_token(request.method) || request.target.empty()) {
        return 400;
    }
    // Ban hoc nay chi nhan target dang /path?query.
    if (request.target[0] != '/') {
        return 400;
    }
    for (unsigned char ch : request.target) {
        if (ch <= 32 || ch >= 127 || ch == '#') {
            return 400;
        }
    }
    if (request.version != "HTTP/1.1") {
        return 505;
    }

    bool has_host = false;
    bool has_length = false;
    bool has_transfer_encoding = false;
    bool has_expect = false;
    std::size_t position = line_end + 2;

    // Doc tung header.
    while (position < header_end) {
        const std::size_t end = data.find("\r\n", position);
        const std::string line = data.substr(position, end - position);
        position = end + 2;

        const std::size_t colon = line.find(':');
        if (colon == std::string::npos) {
            return 400;
        }
        const std::string name = line.substr(0, colon);
        if (!is_token(name)) {
            return 400;
        }
        const std::string key = lowercase(name);
        const std::string value = trim(line.substr(colon + 1));

        for (unsigned char ch : value) {
            if ((ch < 32 && ch != '\t') || ch == 127) {
                return 400;
            }
        }

        if (key == "host") {
            if (has_host || value.empty()) {
                return 400;
            }
            for (unsigned char ch : value) {
                if (ch <= 32 || ch >= 127 ||
                    ch == '/' || ch == '?' || ch == '#' || ch == '@' ||
                    ch == ',' || ch == '\\') {
                    return 400;
                }
            }
            request.host = value;
            has_host = true;
        } else if (key == "content-length") {
            if (has_length) {
                return 400;
            }
            if (value.empty() || value.find_first_not_of("0123456789") !=
                                 std::string::npos) {
                return 400;
            }
            if (!parse_length(value, request.content_length)) {
                return 413;
            }
            has_length = true;
        } else if (key == "transfer-encoding") {
            if (value.empty()) {
                return 400;
            }
            has_transfer_encoding = true;
        } else if (key == "expect") {
            has_expect = true;
        }
    }

    if (!has_host) {
        return 400;
    }
    if (has_transfer_encoding && has_length) {
        return 400;
    }
    if (has_transfer_encoding) {
        return 501;
    }
    if (has_expect) {
        return 417;
    }

    // Headers da het, tiep tuc nhan du body.
    const std::size_t body_start = header_end + 4;
    while (data.size() - body_start < request.content_length) {
        const std::string chunk = client.receive_chunk();
        if (chunk.empty()) {
            return 400;
        }
        data += chunk;
    }
    request.body = data.substr(body_start, request.content_length);
    return 200;
}

static const char* status_text(int status) {
    switch (status) {
        case 200: return "OK";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 413: return "Content Too Large";
        case 417: return "Expectation Failed";
        case 431: return "Request Header Fields Too Large";
        case 501: return "Not Implemented";
        case 505: return "HTTP Version Not Supported";
        default: return "Internal Server Error";
    }
}

std::string make_http_response(
    int status,
    const std::string& body,
    bool head_only
) {
    // Tao Date theo gio GMT. Chuong trinh giu locale C mac dinh.
    const std::time_t now = std::time(nullptr);
    const std::tm* utc = std::gmtime(&now);
    char date[64]{};
    if (utc != nullptr) {
        std::strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", utc);
    }

    std::string response =
        "HTTP/1.1 " + std::to_string(status) + " " + status_text(status) + "\r\n";
    if (date[0] != '\0') {
        response += std::string("Date: ") + date + "\r\n";
    }
    response += "Content-Type: text/plain; charset=utf-8\r\n";
    response += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    response += "Connection: close\r\n";
    if (status == 405) {
        response += "Allow: GET, HEAD\r\n";
    }
    response += "\r\n";
    if (!head_only) {
        response += body;
    }
    return response;
}