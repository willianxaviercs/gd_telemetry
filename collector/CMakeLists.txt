cmake_minimum_required(VERSION 3.16)
project(collector CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(collector src/main.cpp)
target_link_libraries(collector PRIVATE sqlite3)
