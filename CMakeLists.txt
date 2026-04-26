cmake_minimum_required(VERSION 3.16)
project(telemetry CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# libraries
add_subdirectory(libs/cpp)

# applications
add_subdirectory(apps/edge-daemon)
add_subdirectory(apps/stream-consumer)

