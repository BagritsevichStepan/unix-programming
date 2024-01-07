project(Unix_IPC)

add_executable(sysv_msg_queue sysv_msg_queue.cpp)
add_executable(posix_msg_queue posix_msg_queue.cpp)
add_executable(sysv_semaphore sysv_semaphore.cpp)
add_executable(posix_semaphore posix_semaphore.cpp)
