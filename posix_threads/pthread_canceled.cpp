#include <iostream>
#include <pthread.h>
#include<unistd.h>

void* JoinThreadFunction(void* arg) {
    std::cout << "Join Thread: " << __LINE__ << std::endl;

    int old_state;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &old_state);

    std::cout << "Join Thread: " << __LINE__ << std::endl;

    sleep(10);

    std::cout << "Join Thread: " << __LINE__ << std::endl;

    int state;
    pthread_setcancelstate(old_state, &state);

    std::cout << "Join Thread: " << __LINE__ << std::endl;

    return static_cast<void*>(new int{10});
}

void* DetachThreadFunction(void* arg) {
    std::cout << "Detach Thread: " << __LINE__ << std::endl;

    auto join_thread = static_cast<pthread_t*>(arg);
    void* return_value = static_cast<void*>(new int{10});

    std::cout << "Detach Thread: " << __LINE__ << std::endl;

    sleep(5);

    std::cout << "Detach Thread: " << __LINE__ << std::endl;

    int status = pthread_join(*join_thread, &return_value);
    if (status) {
        std::cerr << "Detach Thread: Failed to join thread (thread: " << *join_thread << "): " << status << std::endl;
        return return_value;
    }

    std::cout << "Detach Thread: " << __LINE__ << std::endl;

    if (return_value == PTHREAD_CANCELED) {
        std::cout << "Detach Thread: Join thread was canceled" << std::endl;
    }
    return return_value;
}

bool RunJoinThread(pthread_t* join_thread) {
    int status = pthread_create(join_thread, nullptr, &JoinThreadFunction, nullptr);
    if (status) {
        std::cerr << "Failed to run join thread (thread: " << *join_thread << "): " << status << std::endl;
        return false;
    }
    return true;
}

bool RunDetachThread(pthread_t* detach_thread, pthread_attr_t* detach_thread_attr, pthread_t* join_thread) {

    int status = pthread_attr_setdetachstate(detach_thread_attr, PTHREAD_CREATE_DETACHED);
    if (status) {
        std::cerr << "Failed to set detach state (thread: " << *detach_thread << "): " << status << std::endl;
        return false;
    }

    status = pthread_create(detach_thread, detach_thread_attr, &DetachThreadFunction, static_cast<void*>(join_thread));
    if (status) {
        std::cerr << "Failed to run detach thread (thread: " << *detach_thread << "): " << status << std::endl;
        return false;
    }

    return true;
}

int main() {
    pthread_t join_thread;

    auto detach_thread_attr = new pthread_attr_t{};
    pthread_attr_init(detach_thread_attr);
    pthread_t detach_thread;

    if (!RunJoinThread(&join_thread)) {
        return -1;
    }

    if (!RunDetachThread(&detach_thread, detach_thread_attr, &join_thread)) {
        return -1;
    }

    sleep(1);

    pthread_cancel(join_thread);

    sleep(15);

    pthread_attr_destroy(detach_thread_attr);

    return 0;
}
