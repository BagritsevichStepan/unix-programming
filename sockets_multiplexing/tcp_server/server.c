#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include "error.h"

#define MY_PORT "3490"

#define BACKLOG 20

void sigchld_handler(int s) {
    (void)s;

    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

void reset_sigaction() {
    struct sigaction sa;

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("Server: Failed to set sigaction\n");
        exit(1);
    }
}

struct addrinfo* initialize_address() {
    struct addrinfo addr;

    memset(&addr, 0, sizeof addr);
    addr.ai_family = AF_UNSPEC;
    addr.ai_socktype = SOCK_STREAM;
    addr.ai_flags = AI_PASSIVE;

    struct addrinfo* server_address;
    int status = getaddrinfo(NULL, MY_PORT, &addr, &server_address);
    if (status) {
        print_error("Server: Failed during getaddrinfo:", status);
        exit(0);
    }
    return server_address;
}

int get_socket_fd(struct addrinfo* server_address) {
    int yes = 1;

    int socket_fd = -1;
    for (struct addrinfo* cur = server_address; cur != NULL; cur = cur->ai_next) {
        socket_fd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
        if (socket_fd == -1) {
            print_error("Server: Failed to open the server socket:", errno);
            continue;
        }

        if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            print_error("Server: Failed to reuse the server socket:", errno);
            exit(1);
        }

        if (bind(socket_fd, cur->ai_addr, cur->ai_addrlen) == -1) {
            close(socket_fd);
            print_error("Server: Failed to bind the server socket:", errno);
            continue;
        }

        break;
    }

    if (socket_fd == -1) {
        perror("Server: Socket is not found\n");
        exit(1);
    }

    return socket_fd;
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

int main() {
    char server_message[] = "Hello world!";

    struct addrinfo *server_address = initialize_address();
    int socket_fd = get_socket_fd(server_address);
    freeaddrinfo(server_address);

    listen(socket_fd, BACKLOG);

    reset_sigaction();

    struct sockaddr_storage client_socket_addr;
    socklen_t socklen = sizeof(client_socket_addr);

    while (1) {
        int copy_socket = accept(socket_fd, (struct sockaddr *) &client_socket_addr, &socklen);

        if (copy_socket == -1) {
            print_error("Server: Failed to accept connection:", errno);
            continue;
        }

        uint32_t ip_len = get_addr_len((struct sockaddr *) &client_socket_addr);
        char ip[ip_len];
        void* addr_in = get_addr_in((struct sockaddr *) &client_socket_addr);
        if (inet_ntop(client_socket_addr.ss_family, addr_in, ip, ip_len) != NULL) {
            printf("Server: Gon connection from %s\n", ip);
        } else {
            print_error("Server: Failed to get the client ip address:", errno);
        }


        int process = fork();
        if (process == -1) {
            print_error("Server: Failed to create a new process:", errno);
        }

        if (process == 0) {
            close(socket_fd);

            if (send(copy_socket, server_message, sizeof(server_message), 0) == -1) {
                print_error("Server: Failed to send a message:", errno);
            }

            close(copy_socket);
            exit(0);
        }

        close(copy_socket);
    }
    return 0;
}
