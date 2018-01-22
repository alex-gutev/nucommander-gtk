cmake_minimum_required(VERSION 2.6)
project(nucommander)

add_executable(nucommander main.cpp)

install(TARGETS nucommander RUNTIME DESTINATION bin)
