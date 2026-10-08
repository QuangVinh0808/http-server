cmake_minimum_required(VERSION 3.20)

project(HttpServer LANGUAGES CXX)

add_executable(server
    src/main.cpp
    src/net/socket.cpp
)

target_compile_features(server PRIVATE cxx_std_17)

target_include_directories(server PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(server PRIVATE ws2_32)

if(MSVC)
    target_compile_options(server PRIVATE /W4)
else()
    target_compile_options(server PRIVATE -Wall -Wextra -Wpedantic)
endif()