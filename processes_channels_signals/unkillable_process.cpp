#include <signal.h>

int main() {
    /*
     * ANSI Implementation:
     * signal(SIGTERM, SIG_IGN);
     * signal(SIGINT, SIG_IGN);
    */

    struct sigaction new_action{};
    new_action.sa_handler = SIG_IGN;
    sigfillset(&new_action.sa_mask);

    struct sigaction old_action{};

    sigaction(SIGTERM, &new_action, &old_action);
    sigaction(SIGINT, &new_action, &old_action);

    while (true) {}
    return 0;
}
