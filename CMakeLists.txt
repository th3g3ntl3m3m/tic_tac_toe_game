cmake_minimum_required(VERSION 3.10)
project(TicTacToe VERSION 1.0 LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -g")
else()
  message(STATUS "Debug build: debug symbols are included by default")
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

configure_file(
    ${CMAKE_SOURCE_DIR}/version.in
    ${CMAKE_BINARY_DIR}/version.hpp
    @ONLY
)

add_executable(TicTacToe src/main.cpp)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  if(UNIX)
    add_custom_command(TARGET TicTacToe POST_BUILD
      COMMAND ${CMAKE_OBJCOPY} --only-keep-debug $<TARGET_FILE:TicTacToe> $<TARGET_FILE:TicTacToe>.debug
      COMMAND ${CMAKE_OBJCOPY} --strip-debug $<TARGET_FILE:TicTacToe>
      COMMAND ${CMAKE_STRIP} --strip-unneeded $<TARGET_FILE:TicTacToe>
      COMMENT "Separating debug symbols into separate file: <executable>.debug")
  endif()
endif()
