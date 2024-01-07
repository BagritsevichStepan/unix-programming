project(Posix_Threads)

add_executable(easy_pthreads easy_pthreads.cpp)
add_executable(cancel_pthreads pthread_canceled.cpp)
add_executable(pthread_mutex pthread_mutex.cpp)
add_executable(wait_pthread wait_pthread.cpp)