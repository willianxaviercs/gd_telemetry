add_library(core STATIC
    src/redis_stream_publisher.cpp
    src/sqlite_device_store.cpp
)

target_include_directories(core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/
)


target_link_libraries(core
    PUBLIC
        hiredis
        sqlite3
)

