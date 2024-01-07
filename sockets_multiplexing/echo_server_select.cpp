#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <netdb.h>
#include <sys/select.h>
#include <unistd.h>

#define BACKLOG 1024
#define PORT "12345"

#define ERROR (-1)
#define EXIT_ERROR (-1)

namespace net::utils {
    void print_errno(const std::string &message) {
        std::cerr << message << ": " << message << std::endl;
    }

    int set_nonblock(int socket_fd) {
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

    int get_max_socket(int server_socket, std::set<int>& open_sockets) {
        int max_socket = server_socket;
        if (!open_sockets.empty()) {
            max_socket = std::max(max_socket, *open_sockets.rbegin());
        }
        return max_socket;
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
    static constexpr size_t kBufferSize = 1024;
    char buffer[kBufferSize];

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

    int accept_new_connection(int server_socket, std::set<int>& open_sockets) {
        int new_socket = accept(server_socket, nullptr, nullptr);

        if (new_socket != ERROR) {
            utils::set_nonblock(new_socket);
            open_sockets.insert(new_socket);
        } else {
            utils::print_errno("Server: Failed to accept new connection");
        }

        return new_socket;
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

    net::utils::set_nonblock(server_socket);

    if (listen(server_socket, BACKLOG) == ERROR) {
        net::utils::print_errno("Server: Failed to listen the socket " + std::to_string(server_socket));
        return EXIT_ERROR;
    }

    std::set<int> open_sockets;
    while (true) {
        fd_set set;
        FD_ZERO(&set);

        FD_SET(server_socket, &set);
        for (auto& socket : open_sockets) {
            FD_SET(socket, &set);
        }

        int max_socket = net::utils::get_max_socket(server_socket, open_sockets);

        if (select(max_socket + 1, &set, nullptr, nullptr, nullptr) == ERROR) {
            if (errno == EAGAIN) {
                continue;
            }
            net::utils::print_errno("Server: Failure during select");
            break;
        }

        std::vector<int> closed_sockets;
        for (auto& socket : open_sockets) {
            if (FD_ISSET(socket, &set)) {
                if (!net::echo(socket)) {
                    close(socket);
                    closed_sockets.push_back(socket);
                }
            }
        }

        for (auto& socket : closed_sockets) {
            open_sockets.erase(socket);
        }

        if (FD_ISSET(server_socket, &set) && net::accept_new_connection(server_socket, open_sockets) == ERROR) {
            break;
        }
    }
    return 0;
}
