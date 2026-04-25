# dependencies
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

# --- core ---
add_library(core STATIC
  src/sqlite_device_store.cpp
)

target_include_directories(core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(core
  PUBLIC
    unofficial::sqlite3::sqlite3
)

# --- proto ---
add_library(proto STATIC
  src/device_event.pb.cc
)

target_include_directories(proto
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(proto
  PUBLIC
    protobuf::libprotobuf
)
