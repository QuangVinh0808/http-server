#ifndef __HTTP_PARSER_H__
#define __HTTP_PARSER_H__

#include "http_request.h"
#include "net/socket.h"

int read_http_request(const net::Socket& socket, HttpRequest& request); // Tra ve ma trang thai HTTP
std::string make_http_response(int status, const std::string& body, bool head_only = false);

#endif // __HTTP_PARSER_H__