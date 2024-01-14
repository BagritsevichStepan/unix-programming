project(Parallel_Programming)

add_executable(prefix_sum prefix_sum.cpp)
add_executable(matrix_det matrix_det.cpp)

target_link_libraries(prefix_sum ${CONAN_LIBS})
target_link_libraries(matrix_det ${CONAN_LIBS})
