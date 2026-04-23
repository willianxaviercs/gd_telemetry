add_executable(edge-daemon src/main.cpp)

target_link_libraries(edge-daemon PRIVATE
    PRIVATE
        core
)

set_target_properties(edge-daemon PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/edge-daemon"
)
