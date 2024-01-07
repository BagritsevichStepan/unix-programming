#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*struct in_addr {
    uint32_t s_addr;
};

struct sockaddr_in {
    short int sin_family; // IPv4, IPv6
    unsigned short int sin_port; // Port
    struct in_addr sin_addr; // IP address
    unsigned char sin_zero[8];
};


struct in6_addr {
    unsigned char s6_addr[16];
};

struct sockaddr_in6 {
    uint16_t sin6_family; // IPv4, IPv6
    uint16_t sin6_port; // Port
    uint32_t sin6_flowinfo; // IPv6 info
    struct in6_addr sin6_addr; // IP address
    uint32_t sin6_scope_id; //Scope id
};

// Can store sockaddr_in and sockaddr_in6
struct sockaddr_storage {
    uint16_t ss_family;
    char __ss_pad1[4];
    char __ss_pad2[16];
};*/

void string_to_sockaddr() {
    struct sockaddr_in sa4;
    struct sockaddr_in6 sa6;

    inet_pton(AF_INET, "192.0.2.1", &sa4.sin_addr);
    inet_pton(AF_INET6, "2001:db8:63b3:1::3490", &sa6.sin6_addr);
}

void sockaddr_to_string() {
    char ip4[INET_ADDRSTRLEN];
    struct sockaddr_in initialized_sa4;

    inet_ntop(AF_INET, &initialized_sa4.sin_addr, ip4, INET_ADDRSTRLEN);

    printf("IPv4 address is: %s\n", ip4);


    char ip6[INET6_ADDRSTRLEN];
    struct sockaddr_in6 initialized_sa6;

    inet_ntop(AF_INET6, &initialized_sa6.sin6_addr, ip6, INET6_ADDRSTRLEN);

    printf("IPv6 address is: %s\n", ip6);
}

int main(int args, char** argv) {
    string_to_sockaddr();
    printf("Sockaddr end");
    return 0;
}
