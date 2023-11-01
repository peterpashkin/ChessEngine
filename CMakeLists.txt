cmake_minimum_required(VERSION 3.26)
project(ChessEngineGit)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-no-pie -O3 -march=native")

include_directories(.)

add_executable(ChessEngineGit
        Board.cpp
        Board.h
        Helper.cpp
        Helper.h
        main.cpp)
