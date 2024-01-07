#include <iostream>
#include <vector>
#include <pthread.h>
#include <unistd.h>
#include <cassert>


namespace {
    std::vector<int> a{};

    constexpr int kOkStatus = std::numeric_limits<int>::max();
    constexpr int kBadStatus = std::numeric_limits<int>::min();
    constexpr int kInvalidElement = std::numeric_limits<int>::min();

    void* Producer(void* arg) {
        auto rw_lock = static_cast<pthread_rwlock_t*>(arg);
        int status;

        for (int i = 0; i < 10; i++) {
            if ((status = pthread_rwlock_wrlock(rw_lock))) {
                std::cerr << "Producer: failure during lock: " << status << std::endl;
                i--;
                continue;
            }

            a.push_back(i);

            if ((status = pthread_rwlock_unlock(rw_lock))) {
                std::cerr << "Producer: failure during unlock: " << status << std::endl;
                continue;
            }
        }
        return static_cast<void*>(new int{kOkStatus});
    }

    void* Consumer(void* arg) {
        auto rw_lock = static_cast<pthread_rwlock_t*>(arg);
        int status;

        int read_elements_number = 0;
        while (read_elements_number < 10) {
            int read_element = kInvalidElement;

            if ((status = pthread_rwlock_rdlock(rw_lock))) {
                std::cerr << "Consumer: failure during lock: " << status << std::endl;
                continue;
            }

            if (!a.empty()) {
                read_element = a.back();
                a.pop_back();
            }

            if ((status = pthread_rwlock_unlock(rw_lock))) {
                std::cerr << "Producer: failure during unlock: " << status << std::endl;
                continue;
            }

            if (read_element != kInvalidElement) {
                std::cout << "Consumer: read element " << read_element << std::endl;
                read_elements_number++;
            }
        }
        return static_cast<void*>(new int{kOkStatus});
    }

    bool RunProducer(pthread_t* producer, pthread_rwlock_t* rwlock) {
        int status = pthread_create(producer, nullptr, &Producer, static_cast<void*>(rwlock));
        if (status) {
            std::cerr << "Failed to run producer" << status << std::endl;
            return false;
        }
        return true;
    }

    bool RunConsumer(pthread_t* consumer, pthread_rwlock_t* rwlock) {
        int status = pthread_create(consumer, nullptr, &Consumer, static_cast<void*>(rwlock));
        if (status) {
            std::cerr << "Failed to run consumer" << status << std::endl;
            return false;
        }
        return true;
    }

    void ConsumerAndProducer() {
        a.clear();

        int status;
        pthread_t producer;
        pthread_t consumer;

        pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

        if (!RunProducer(&producer, &rwlock)) {
            return;
        }

        if (!RunConsumer(&consumer, &rwlock)) {
            return;
        }

        void* return_value_producer = static_cast<void*>(new int{kBadStatus});
        if ((status = pthread_join(producer, &return_value_producer))) {
            std::cerr << "Failed to join producer: " << status << std::endl;
        } else {
            assert(*static_cast<int*>(return_value_producer) == kOkStatus);
        }

        void* return_value_consumer = static_cast<void*>(new int{kBadStatus});
        if ((status = pthread_join(consumer, &return_value_consumer))) {
            std::cerr << "Failed to join consumer: " << status << std::endl;
        } else {
            assert(*static_cast<int*>(return_value_consumer) == kOkStatus);
        }
    }

    int counter = 0;
    constexpr int kMutexThreadsNumber = 5;

    void* MutexThread(void* arg) {
        auto mutex = static_cast<pthread_mutex_t*>(arg);
        int status;

        if ((status = pthread_mutex_lock(mutex))) {
            std::cerr << "MutexThread (" << pthread_self() << "): failed to lock mutex: " << status << std::endl;
            return static_cast<void*>(new int{kBadStatus});
        }

        std::cout << "MutexThread (" << pthread_self() << "): counter=" << counter++ << std::endl;

        if ((status = pthread_mutex_unlock(mutex))) {
            std::cerr << "MutexThread (" << pthread_self() << "): failed to unlock mutex: " << status << std::endl;
            return static_cast<void*>(new int{kBadStatus});
        }

        return static_cast<void*>(new int{kOkStatus});
    }

    void RunMutexThreads() {
        counter = 0;

        pthread_t threads[kMutexThreadsNumber];
        pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
        int status;

        for (int i = 0; i < kMutexThreadsNumber; i++) {
            if ((status = pthread_create(&threads[i], nullptr, &MutexThread, static_cast<void*>(&mutex)))) {
                std::cerr << "Failed to run mutex thread (" << i + 1 << "): " << status << std::endl;
                return;
            }
        }

        void* return_value = static_cast<void*>(new int{kBadStatus});
        for (int i = 0; i < kMutexThreadsNumber; i++) {
            if ((status = pthread_join(threads[i], &return_value))) {
                std::cerr << "Failed to join thread (" << i + 1 << "): " << status << std::endl;
            } else {
                assert(*static_cast<int *>(return_value) == kOkStatus);
            }
        }

        assert(counter == kMutexThreadsNumber);
    }

}

int main() {
    RunMutexThreads();
    ConsumerAndProducer();
    return 0;
}
