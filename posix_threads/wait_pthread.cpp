#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <cassert>

namespace {

    constexpr int kOkStatus = std::numeric_limits<int>::max();
    constexpr int kBadStatus = std::numeric_limits<int>::min();
    constexpr int kInvalidElement = std::numeric_limits<int>::min();

    int state = 0;
    pthread_mutex_t mutex;

    void* WaitThread1(void* arg) {
        std::cout << "WaitThread1: " << __LINE__ << std::endl;
        auto cv = static_cast<pthread_cond_t*>(arg);
        int status;

        sleep(5);

        state = 1;
        if ((status = pthread_cond_signal(cv))) {
            std::cerr << "WaitThread1: failed during cv broadcast: " << status << std::endl;
            return static_cast<void*>(new int{kBadStatus});
        }

        sleep(1);

        std::cout << "WaitThread1: " << __LINE__ << std::endl;
        return static_cast<void*>(new int{kOkStatus});
    }

    void* WaitThread2(void* arg) {
        std::cout << "WaitThread2: " << __LINE__ << std::endl;
        auto cv = static_cast<pthread_cond_t*>(arg);
        int status;

        assert(state == 0);

        if ((status = pthread_cond_wait(cv, &mutex))) {
            std::cerr << "WaitThread2: failed during wait: " << status << std::endl;
            return static_cast<void*>(new int{kBadStatus});
        }

        assert(state == 1);

        std::cout << "WaitThread2: " << __LINE__ << std::endl;
        return static_cast<void*>(new int{kOkStatus});
    }

    void RunWaitThreads() {
        state = 0;
        mutex = PTHREAD_MUTEX_INITIALIZER;
        int status;

        pthread_t wait_threads[2];
        pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

        if ((status = pthread_create(&wait_threads[0], nullptr, &WaitThread1, static_cast<void*>(&cv)))) {
            std::cerr << "Failed to run first wait thread: " << status << std::endl;
            return;
        }

        if ((status = pthread_create(&wait_threads[1], nullptr, &WaitThread2, static_cast<void*>(&cv)))) {
            std::cerr << "Failed to run second wait thread: " << status << std::endl;
            return;
        }

        void* return_value = static_cast<void*>(new int{kBadStatus});
        for (int i = 0; i < 2; i++) {
            if ((status = pthread_join(wait_threads[i], &return_value))) {
                std::cerr << "Failed to join wait thread (" << i + 1 << "): " << status << std::endl;
            } else {
                assert(*static_cast<int*>(return_value) == kOkStatus);
            }
        }
    }
}

int main() {
    RunWaitThreads();
    return 0;
}