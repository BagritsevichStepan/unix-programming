#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <cassert>

void* ThreadFunction(void* arg) {
    int* state = static_cast<int*>(arg);
    std::cout << "Pid: " << getpid() << " Thread: " << pthread_self();
    std::cout << " State: " << *state << std::endl;
    (*state)++;
    return static_cast<void*>(state);
}

int main() {
    int status;
    pthread_t first_thread;
    pthread_t second_thread;

    int state = 1;

    status = pthread_create(&first_thread, nullptr, &ThreadFunction, static_cast<void*>(&state));
    if (status) {
        std::cerr << "Failed during pthread_create(1): " << state << std::endl;
        return -1;
    }

    status = pthread_create(&second_thread, nullptr, &ThreadFunction, static_cast<void*>(&state));
    if (status) {
        std::cerr << "Failed during pthread_create(2): " << state << std::endl;
        return -1;
    }

    void* first_thread_state = static_cast<void*>(new int{0});

    status = pthread_join(first_thread, &first_thread_state);
    if (status) {
        std::cerr << "Failed during pthread_join(1): " << state << std::endl;
        return -1;
    }

    void* second_thread_state = static_cast<void*>(new int{0});
    status = pthread_join(second_thread, &second_thread_state);
    if (status) {
        std::cerr << "Failed during pthread_join(2): " << state << std::endl;
        return -1;
    }

    assert(*(static_cast<int*>(second_thread_state)) == 3);
    return 0;
}
