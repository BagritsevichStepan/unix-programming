#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/epoll.h>

#define PORT "12347"

#define ERROR (-1)
#define EXIT_ERROR (-1)

namespace net::utils {
    void print_errno(const std::string &message) {
        std::cerr << message << ": " << errno << std::endl;
    }

    int set_nonblock(const int socket_fd) {
        int flags;
#if defined(O_NONBLOCK)
        if ((flags = fcntl(socket_fd, F_GETFL, 0)) == ERROR) {
            flags = 0;
        }
        return fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
#else
        flags = 1;
        return ioctl(socket_fd, FIONBIO, &flags);
#endif
    }
}

namespace net::init {

    addrinfo *initialize_addr_info() {
        addrinfo addr_info{};
        addr_info.ai_family = AF_UNSPEC;
        addr_info.ai_socktype = SOCK_STREAM;
        addr_info.ai_flags = AI_PASSIVE;

        addrinfo *server_addr_info;
        const int status = getaddrinfo(nullptr, PORT, &addr_info, &server_addr_info);

        if (status == ERROR) {
            std::cerr << "Server failure during getaddrinfo: " << gai_strerror(status) << std::endl;
            return nullptr;
        }

        return server_addr_info;
    }

    int get_socket_fd(addrinfo *addr_info) {
        int yes = 1;
        int socket_fd = ERROR;
        for (addrinfo *current = addr_info; current; current = current->ai_next) {
            socket_fd = socket(current->ai_family, current->ai_socktype, current->ai_protocol);

            if (socket_fd == ERROR) {
                utils::print_errno("Server: Failed to connect to the socket");
                continue;
            }

            if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == ERROR) {
                utils::print_errno("Server: Failed to reuse the socket");
                return ERROR;
            }

            if (bind(socket_fd, current->ai_addr, current->ai_addrlen) == ERROR) {
                close(socket_fd);
                utils::print_errno("Server: Failed to bind the socket");
                continue;
            }

            break;
        }
        if (socket_fd == ERROR) {
            std::cerr << "Server: Failed to find socket" << std::endl;
        }
        return socket_fd;
    }
}

namespace net {
    constexpr size_t kBufferSize = 1024;
    char buffer[kBufferSize];

    constexpr size_t kEventsSize = SOMAXCONN;

    bool echo(int socket) {
        size_t received_message_size = recv(socket, &buffer, kBufferSize, 0);
        if (received_message_size <= 0) {
            if (errno != EAGAIN) {
                utils::print_errno("Server: Failed to receive a message");
            }
            return errno == EAGAIN;
        }

        if (send(socket, &buffer, received_message_size, 0) == ERROR) {
            utils::print_errno("Server: Failed to send the message");
        }

        return true;
    }

    int add_new_connection(const int socket, const int epoll) {
        utils::set_nonblock(socket);

        epoll_event event;
        event.data.fd = socket;
        event.events = EPOLLIN;

        if (epoll_ctl(epoll, EPOLL_CTL_ADD, socket, &event) == ERROR) {
            net::utils::print_errno(
                    "Server: Failed to add the socket " + std::to_string(socket) +
                    " to the epoll " + std::to_string(epoll)
            );
            return ERROR;
        }

        return socket;
    }

    void accept_new_connection(int server_socket, const int epoll) {
        int new_socket = accept(server_socket, nullptr, nullptr);

        if (new_socket != ERROR) {
            add_new_connection(new_socket, epoll);
        } else {
            utils::print_errno("Server: Failed to accept new connection");
        }
    }

}

int main() {
    addrinfo* server_addr_info = net::init::initialize_addr_info();
    if (server_addr_info == nullptr) {
        return EXIT_ERROR;
    }

    int server_socket = net::init::get_socket_fd(server_addr_info);
    freeaddrinfo(server_addr_info);
    if (server_socket == ERROR) {
        return EXIT_ERROR;
    }

    const int epoll = epoll_create1(0);
    if (net::add_new_connection(server_socket, epoll) == ERROR) {
        return EXIT_ERROR;
    }

    if (listen(server_socket, SOMAXCONN) == ERROR) {
        net::utils::print_errno("Server: Failed to listen the socket " + std::to_string(server_socket));
        return EXIT_ERROR;
    }

    while (true) {
        epoll_event events[net::kEventsSize];

        const int events_number = epoll_wait(epoll, events, net::kEventsSize, -1);
        if (events_number == ERROR) {
            net::utils::print_errno("Server: Failure during epoll wait");
            break;
        }

        for (int i = 0; i < events_number; i++) {
            const int socket = events[i].data.fd;

            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
                close(socket);
                if (socket == server_socket) {
                    std::cerr << "Server socket was closed" << std::endl;
                    return EXIT_ERROR;
                }
            }

            if (socket == server_socket) {
                net::accept_new_connection(server_socket, epoll);
            } else if (!net::echo(socket)) {
                close(socket);
            }
        }
    }
    return 0;
}
