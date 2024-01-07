#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Number of arguments must be 2");
        exit(1);
    }

    int status;

    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *res;

    if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) {
        fprintf(stderr, "Failed during getaddrinfo: %s\n", gai_strerror(status));
        exit(1);
    }

    char ip_address[INET6_ADDRSTRLEN];

    printf("IP addresses for %s\n", argv[1]);
    for (struct addrinfo *cur = res; cur != NULL; cur = cur->ai_next) {
        char* ip_version;
        void* addr;

        if (cur->ai_family == AF_INET) { // IPv4
            struct sockaddr_in* sockaddr = (struct sockaddr_in*)cur->ai_addr;
            ip_version = "IPv4";
            addr = &sockaddr->sin_addr;
        } else { //IPv6
            struct sockaddr_in6* sockaddr = (struct sockaddr_in6*)cur->ai_addr;
            ip_version = "IPv6";
            addr = &sockaddr->sin6_addr;
        }

        inet_ntop(cur->ai_family, &addr, ip_address, sizeof ip_address);
        printf(" %s: %s\n", ip_version, ip_address);
    }

    freeaddrinfo(res);
    return 0;
}