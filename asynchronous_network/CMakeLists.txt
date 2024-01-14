project(Asynchronous_Network)

add_executable(libevent libevent_server.c)
add_executable(libev libev_server.c)
add_executable(libuv libuv_server.c)
add_executable(asio boost_asio_server.cpp)

target_link_libraries(libevent ${CONAN_LIBS})
target_link_libraries(libev ${CONAN_LIBS})
target_link_libraries(libuv ${CONAN_LIBS})
target_link_libraries(asio ${CONAN_LIBS})
