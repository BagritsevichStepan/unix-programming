cmake_minimum_required(VERSION 3.9)
project(Unix_Programming)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

include(conanbuildinfo.cmake)
conan_basic_setup()

add_subdirectory(containers_allocators/)
add_subdirectory(sockets_multiplexing/)
add_subdirectory(asynchronous_network/)
add_subdirectory(processes_channels_signals/)
add_subdirectory(unix_ipc/)
add_subdirectory(posix_threads/)
add_subdirectory(parallel_programming/)
