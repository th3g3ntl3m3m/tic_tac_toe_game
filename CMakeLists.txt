cmake_minimum_required(VERSION 3.10)
project(TicTacToe VERSION 1.0 LANGUAGES CXX)

include_directories(${CMAKE_SOURCE_DIR}/include)

configure_file(
    ${CMAKE_SOURCE_DIR}/version.in
    ${CMAKE_BINARY_DIR}/version.hpp
    @ONLY
)

add_executable(TicTacToe src/main.cpp)
