#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "error.h"

#define MY_PORT "3490"

#define MAX_DATA_SIZE 200

struct addrinfo* initialize_address(const char* hostname) {
    struct addrinfo addr;

    memset(&addr, 0, sizeof(addr));
    addr.ai_family = AF_UNSPEC;
    addr.ai_socktype = SOCK_STREAM;
    addr.ai_protocol = AI_PASSIVE;

    struct addrinfo* client_address;
    int status = getaddrinfo(hostname, MY_PORT, &addr, &client_address);
    if (status) {
        print_error("Client: Failed during getaddrinfo:", status);
        exit(0);
    }

    return client_address;
}

void* get_addr_in(struct sockaddr* sockaddr) {
    if (sockaddr->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sockaddr)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sockaddr)->sin6_addr);
}

uint32_t get_addr_len(struct sockaddr* sockaddr) {
    return sockaddr->sa_family == AF_INET ? INET_ADDRSTRLEN : INET6_ADDRSTRLEN;
}

int get_socket_fd(struct addrinfo* server_address) {
    int socket_fd = -1;
    for (struct addrinfo* cur = server_address; cur != NULL; cur = cur->ai_next) {
        socket_fd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);

        if (socket_fd == -1) {
            print_error("Client: Failed to open the socket:", errno);
            continue;
        }

        if (connect(socket_fd, cur->ai_addr, cur->ai_addrlen) == -1) {
            close(socket_fd);
            print_error("Client: Failed to connect:", errno);
            continue;
        }

        uint32_t ip_len = get_addr_len(cur->ai_addr);
        char ip[ip_len];
        void* addr_in = get_addr_in(cur->ai_addr);

        if (inet_ntop(cur->ai_family, addr_in, ip, ip_len) != NULL) {
            printf("Client: Connecting to %s\n", ip);
        } else {
            print_error("Client: Failed to get the server ip address:", errno);
        }

        break;
    }

    if (socket_fd == -1) {
        perror("Client: No connection was found\n");
        exit(1);
    }

    return socket_fd;
}


int main(int argc, char** argv) {
    if (argc != 2) {
        perror("Client: Enter hostname in arguments\n");
        exit(1);
    }

    struct addrinfo* server_address = initialize_address(argv[1]);
    int socket_fd = get_socket_fd(server_address);
    freeaddrinfo(server_address);

    long received_bytes_number;
    char received_message[MAX_DATA_SIZE + 1];

    if ((received_bytes_number = recv(socket_fd, received_message, MAX_DATA_SIZE, 0)) == -1) {
        print_error("Client: Failed to receive a message from server:", errno);
    } else {
        received_message[received_bytes_number] = '\0';
        printf("Client: Received from server: \"%s\"\n", received_message);
    }

    close(socket_fd);
    return 0;
}