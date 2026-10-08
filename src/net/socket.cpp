#include "net/socket.h"
#include <iostream>
int main(){
    struct sockaddr_in sa;   // IPv4
    struct sockaddr_in6 sa6; // IPv6

    inet_pton(AF_INET, "10.12.110.57", &(sa.sin_addr));
    inet_pton(AF_INET6, "2001:db8:63b3:1::3490", &(sa6.sin6_addr));
    
    
    char ip4[INET_ADDRSTRLEN];
    struct sockaddr_in sa;
    inet_ntop(AF_INET, &(sa.sin_addr), ip4, INET_ADDRSTRLEN);
    std::cout << "IPv4 address: " << ip4 << std::endl;

    char ip6[INET6_ADDRSTRLEN];
    struct sockaddr_in6 sa6;
    inet_ntop(AF_INET6, &(sa6.sin6_addr), ip6, INET6_ADDRSTRLEN);
    std::cout << "IPv6 address: " << ip6 << std::endl;
}
