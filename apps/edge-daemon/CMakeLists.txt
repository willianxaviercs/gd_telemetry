add_executable(edge-daemon
    src/main.cpp
    src/Config.cpp
    src/EventPublisher.cpp
)

find_package(redis++ CONFIG REQUIRED)
find_package(hiredis CONFIG REQUIRED)

target_link_libraries(edge-daemon PRIVATE
    core
    proto
    redis++::redis++_static
    hiredis::hiredis
)

target_link_options(edge-daemon PRIVATE -static)

set_target_properties(edge-daemon PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/edge-daemon"
)

