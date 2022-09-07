cmake_minimum_required(VERSION 3.19)
project(cpp)

set(CMAKE_CXX_STANDARD 98)

include_directories(openssl/include)
link_directories(openssl/lib)
add_executable(http http.cpp)

add_executable(https https.cpp)
target_link_libraries(https ssl crypto)