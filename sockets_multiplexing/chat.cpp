#include <iostream>
#include <set>
#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAX_EVENTS 32

int set_nonblock(int fd) {
    int flags;
#if defined(O_NONBLOCK)
    if (-1 == (flags = fcntl(fd, F_GETFL, 0))) {
        flags = 0;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
    flags = 1;
    return ioctl(fd, FIOBIO, &flags);
#endif
}

std::set<int> slave_sockets;

void SendToAll(char* message) {
    for (const auto& it : slave_sockets) {
        send(it, message, strlen(message), MSG_NOSIGNAL);
    }
}

int main() {
    int master_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in sock_addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(12345);
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(master_socket, (struct sockaddr *)(&sock_addr), sizeof(sock_addr));
    set_nonblock(master_socket);
    listen(master_socket, SOMAXCONN);

    int epoll = epoll_create1(0);

    struct epoll_event event;
    event.data.fd = master_socket;
    event.events = EPOLLIN;

    epoll_ctl(epoll, EPOLL_CTL_ADD, master_socket, &event);

    while (true) {
        struct epoll_event events[MAX_EVENTS];
        int n = epoll_wait(epoll, events, MAX_EVENTS, -1);

        for (unsigned int i = 0; i < n; i++) {
            struct sockaddr_in addr;
            socklen_t addr_size = sizeof(struct sockaddr_in);
            getpeername(events[i].data.fd, (struct sockaddr *)&addr, &addr_size);
            char* clientip = new char[20];
            strcpy(clientip, inet_ntoa(addr.sin_addr));

            char message[1024];
            strcpy(message, clientip);

            if (events[i].data.fd == master_socket) {
                int slave_socket = accept(master_socket, 0, 0);
                set_nonblock(slave_socket);
                slave_sockets.insert(slave_socket);

                struct epoll_event event;
                event.data.fd = slave_socket;
                event.events = EPOLLIN;
                epoll_ctl(epoll, EPOLL_CTL_ADD, slave_socket, &event);
                strcat(message, " connected!");
            } else {
                static char buffer[1024];
                int recv_size = recv(events[i].data.fd, buffer, 1024, MSG_NOSIGNAL);

                if ((recv_size == 0) && (errno != EAGAIN)) {
                    shutdown(events[i].data.fd, SHUT_RDWR);
                    close(events[i].data.fd);
                    slave_sockets.erase(events[i].data.fd);
                    strcat(message, " disconnected!");
                } else if (recv_size > 0) {
                    strcat(message, ": ");
                    buffer[recv_size] = 0;
                    strcat(message, buffer);
                }
            }
            SendToAll(message);
        }
    }
    return 0;
}
