#include <unistd.h>
#include <iostream>
#include <signal.h>

namespace {
    sig_atomic_t sigtermInvoked = 0;

    void ChildHandler(int pid) {
        sigtermInvoked = 1;
        std::cout << "Child. Line:" << __LINE__ << std::endl;
    }

}

int main() {
    const int f = fork();

    if (f == -1) {
        std::cerr << "Error during fork: " << errno << std::endl;
        return -1;
    }

    if (!f) {
        struct sigaction new_action{};
        new_action.sa_handler = &ChildHandler;
        sigfillset(&new_action.sa_mask);

        struct sigaction old_action{};
        sigaction(SIGTERM, &new_action, &old_action);

        while (!sigtermInvoked) {}
        std::cout << "Child. Line:" << __LINE__ << std::endl;
    } else {
        std::cout << "Child pid: " << f << std::endl;
        std::cout << "Parent pid: " << getpid() << std::endl;

        int status;
        waitpid(f, &status, 0);
        std::cout << "Returned status from child: " << status << std::endl;
    }

    return 0;
}
