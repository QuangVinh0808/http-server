


struct addinfo{
    int ai_flags;
    int ai_family;
    int ai_socktype;
    int ai_protocol; // use "0" for any
    size_t ai_addrlen;
    struct sockaddr *ai_addr;
    char *ai_canonname;

    struct addinfo *ai_next;
};

struct sockaddr{
    unsigned short sa_family; // address family, AF_xxx
    char sa_data[14]; // 14 bytes of protocol address
};

// Ipv4 only
struct sockaddr_in{
    unsigned short sin_family; // address family, AF_INET
    unsigned short sin_port; // port in network byte order
    struct in_addr sin_addr; // internet address
    char sin_zero[8]; // padding
};
struct in_addr{
    int32_t s_addr; // 32bit for IPv4 address
};

/*IPv6 only*/
struct sockaddr_in6{
    u_int16_t sin6_family; 
    u_int16_t sin6_port; // port in network byte order
    u_int32_t sin6_flowinfo; // IPv6 flow information
    struct in6_addr sin6_addr; 
    uint32_t sin6_scope_id; // scope id
};
struct in6_addr{
    unsigned char s6_addr[16]; // 128bit for IPv6 address
};

/**/
struct sockaddr_storage{
    sa_family_t ss_family; // address family
    //all is pading
    char __ss_pad1[_SS_PAD1SIZE];
    int64_t __ss_align; 
    char __ss_pad2[_SS_PAD2SIZE]; 
}