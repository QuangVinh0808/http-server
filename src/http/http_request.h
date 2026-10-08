#ifndef __HTTP_REQUEST_H__
#define __HTTP_REQUEST_H__

#include <cstddef>
#include <string>

struct HttpRequest {
    std::string method;
    std::string target;
    std::string version;
    std::string host;
    std::string body;
    std::size_t content_length = 0;
};

#endif // __HTTP_REQUEST_H__