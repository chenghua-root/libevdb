aux_source_directory(. SRC_LIST)

add_executable(libevdbclient ${SRC_LIST})

link_directories (
    ${CMAKE_BINARY_DIR}/src/lib
)

target_link_libraries(libevdbclient
    lib
    -lprotobuf-c
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(libevdbclient Threads::Threads)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
