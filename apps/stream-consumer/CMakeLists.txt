add_executable(stream-consumer 
    src/main.cpp
)

find_package(redis++ CONFIG REQUIRED)
find_package(hiredis CONFIG REQUIRED)
find_package(libpqxx CONFIG REQUIRED)

target_link_libraries(stream-consumer PRIVATE
    core
    proto
    redis++::redis++_static
    hiredis::hiredis
    libpqxx::pqxx
)

target_link_options(stream-consumer PRIVATE -static)

set_target_properties(stream-consumer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/stream-consumer"
)
