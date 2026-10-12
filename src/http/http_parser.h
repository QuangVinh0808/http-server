#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include "http_request.h"
#include "net/socket.h"

int read_http_request(const net::Socket& client, HttpRequest& request,
                      int timeout_ms = 10000);

std::string make_http_response(int status, const std::string& body,
                               bool head_only = false);

#endif