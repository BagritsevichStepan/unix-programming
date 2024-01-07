#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

namespace {

    constexpr size_t kBufferSize = 1024;

    int SocketPairFailed() {
        std::cerr << "Failed during socketpair: " << errno << std::endl;
        return EXIT_FAILURE;
    }

    int ForkFailed() {
        std::cerr << "Failed during fork: " << errno << std::endl;
        return EXIT_FAILURE;
    }

}

int main() {
    int socket_fd[2];

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, socket_fd) == -1) {
        return SocketPairFailed();
    }

    int f = fork();

    if (f == -1) {
        return ForkFailed();
    }

    if (!f) {
        close(socket_fd[1]);

        char buffer[] = "Hello from child";
        if (send(socket_fd[0], &buffer, sizeof(buffer), MSG_NOSIGNAL) == -1) {
            std::cerr << "Child: send failed: " << errno << std::endl;
        }

        close(socket_fd[0]);
    } else {
        close(socket_fd[0]);

        char buffer[kBufferSize];
        ssize_t recv_bytes = recv(socket_fd[1], &buffer, kBufferSize, MSG_NOSIGNAL);

        if (recv_bytes > 0) {
            std::string message{buffer};
            std::cout << "Parent: " << message.substr(0, recv_bytes) << std::endl;
        } else if (recv_bytes < 0) {
            std::cerr << "Parent: recv failed: " << errno << std::endl;
        }

        close(socket_fd[1]);
    }
    return 0;
}
