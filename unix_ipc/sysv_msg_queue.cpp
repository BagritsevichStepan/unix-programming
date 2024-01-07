#include <iostream>
#include <string>
#include <fstream>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/msg.h>
#include <csignal>

namespace {

    const char* kFileName = "msg.temp";

    struct Message {
        long mtype;
        char mtext[80];
    };

    int ForkFailed() {
        std::cerr << "Failed during fork: " << errno << std::endl;
        return EXIT_FAILURE;
    }

    int SndFailed() {
        std::cerr << "Failed during msgsnd: " << errno << std::endl;
        return EXIT_FAILURE;
    }

    int RcvFailed() {
        std::cerr << "Failed during msgrcv: " << errno << std::endl;
        return EXIT_FAILURE;
    }
}

int main() {
    key_t key = ftok(kFileName, 1);
    int msg_id = msgget(key, IPC_CREAT | 0666);

    int f = fork();
    if (f == -1) {
        return ForkFailed();
    }

    if (!f) {
        Message message{10, "Hello from child"};
        if (msgsnd(msg_id, &message, sizeof(message.mtext), 0) == -1) {
            msgctl(msg_id, IPC_RMID, nullptr);
            return SndFailed();
        }
        raise(SIGTERM);
    } else {
        Message message{10L};
        memset(&message, 0, sizeof(message));
        ssize_t bytes = msgrcv(msg_id, static_cast<void*>(&message), sizeof(Message::mtext), 10L, 0);

        if (bytes < 0) {
            msgctl(msg_id, IPC_RMID, nullptr);
            return RcvFailed();
        }

        std::cout << "Parent: " << message.mtext << std::endl;

        int return_value;
        waitpid(f, &return_value, 0);
    }

    msgctl(msg_id, IPC_RMID, nullptr);
    return 0;
}
