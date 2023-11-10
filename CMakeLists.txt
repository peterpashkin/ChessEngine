cmake_minimum_required(VERSION 3.0)

project(ChessEngineGit)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-no-pie -O3 -march=native")

# Find SFML package
find_package(SFML 2.5 COMPONENTS system window graphics REQUIRED)

# Define the source files of your project
set(SOURCES
    Board.cpp
    Board.h
    Helper.cpp
    Helper.h
        main.cpp
)

# Create an executable from your source files
add_executable(ChessEngineGit ${SOURCES})

# Link your executable with SFML libraries
target_link_libraries(ChessEngineGit sfml-graphics sfml-window sfml-system)

# Set the include directories for your project
target_include_directories(ChessEngineGit PRIVATE ${SFML_INCLUDE_DIR})
