#include <iostream>
#include <fstream>
#include <unistd.h>
#include <mqueue.h>
#include <cstring>
#include <sys/wait.h>
#include <sys/fcntl.h>

namespace {

    constexpr size_t kMsgSize = 8192;
    const char* kMqName = "/test.mq";

    int ForkFailed() {
        std::cerr << "Failed during fork: " << errno << std::endl;
        return EXIT_FAILURE;
    }

    int MqOpenFailed() {
        std::cerr << "Failed during mqopen: " << errno << std::endl;
        return EXIT_FAILURE;
    }

    int SndFailed() {
        std::cerr << "Failed during mq_send: " << errno << std::endl;
        return EXIT_FAILURE;
    }

    void RcvFailed() {
        std::cerr << "Failed during msgrcv: " << errno << std::endl;
    }
}

int main() {
    int f = fork();
    if (f == -1) {
        return ForkFailed();
    }

    if (!f) {
        mq_attr attr;
        memset(&attr, 0, sizeof(attr));
        attr.mq_flags = 0;
        attr.mq_maxmsg = 10;
        attr.mq_msgsize = kMsgSize;
        attr.mq_curmsgs = 0;

        mqd_t mq_id = mq_open(kMqName, O_CREAT, 0777, &attr);
        if (mq_id == -1) {
            mq_close(mq_id);
            return MqOpenFailed();
        }

        const char* msg = "Hello from child";
        if (mq_send(mq_id, msg, sizeof(msg), 20) == -1) {
            mq_close(mq_id);
            return SndFailed();
        }

        mq_close(mq_id);
        std::cout << "Child. Line: " << __LINE__ << std::endl;
    } else {
        int return_value;
        waitpid(f, &return_value, 0);
        if (return_value == EXIT_FAILURE) {
            std::cerr << "Parent: error in child" << std::endl;
            return EXIT_FAILURE;
        }

        std::cout << "Parent. Line: " << __LINE__ << std::endl;

        mqd_t mq_id = mq_open(kMqName, O_RDONLY);
        if (mq_id == -1) {
            return MqOpenFailed();
        }

        char msg[kMsgSize];
        memset(&msg, 0, sizeof(msg));
        ssize_t bytes = mq_receive(mq_id, msg, sizeof(msg), nullptr);

        if (bytes == -1) {
            RcvFailed();
        } else {
            std::cout << "Parent: " << msg << std::endl;
        }

        mq_unlink(kMqName);
        mq_close(mq_id);
    }
    return 0;
}
