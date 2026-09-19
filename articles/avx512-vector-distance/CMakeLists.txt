cmake_minimum_required(VERSION 3.20)
project(vectoramp_avx512_vector_distance LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(vector_distance
  src/kernels.cpp
)
target_include_directories(vector_distance PUBLIC include)
target_compile_options(vector_distance PRIVATE -O3 -Wall -Wextra -Wpedantic)

add_executable(kernel_tests tests/kernel_tests.cpp)
target_link_libraries(kernel_tests PRIVATE vector_distance)
target_compile_options(kernel_tests PRIVATE -O2 -Wall -Wextra -Wpedantic)

add_executable(kernel_bench benches/kernel_bench.cpp)
target_link_libraries(kernel_bench PRIVATE vector_distance)
target_compile_options(kernel_bench PRIVATE -O3 -Wall -Wextra -Wpedantic)

enable_testing()
add_test(NAME kernel_tests COMMAND kernel_tests)

