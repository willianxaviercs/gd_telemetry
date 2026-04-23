add_executable(stream-consumer src/main.cpp)

target_link_libraries(stream-consumer PRIVATE
    PRIVATE
        core
)

set_target_properties(stream-consumer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/stream-consumer"
)
