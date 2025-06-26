cmake_minimum_required(VERSION 3.25)

project(irctf)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")

set(APP_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(BLEND2D_DIR "${APP_DIR}/external/blend2d")
set(BLEND2D_NO_JIT TRUE)
include("${BLEND2D_DIR}/CMakeLists.txt")

add_executable(irctf src/irctf.cpp src/irc/network.cpp src/irc/responses.cpp
    src/gui/gui.cpp)
target_link_libraries(irctf blend2d::blend2d SDL3)

IF (NOT WIN32)
    target_link_libraries(irctf fontconfig)
ENDIF()