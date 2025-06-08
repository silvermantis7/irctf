cmake_minimum_required(VERSION 3.25)

project(irctf)

add_executable(irctf src/irctf.cpp src/irc/network.cpp src/irc/responses.cpp)
