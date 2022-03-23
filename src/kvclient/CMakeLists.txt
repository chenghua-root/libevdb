aux_source_directory(. SRC_LIST)

add_executable(kvclient ${SRC_LIST})

link_directories (
    ${CMAKE_BINARY_DIR}/src/lib
)

target_link_libraries(kvclient
    -lprotobuf-c
    lib
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(kvclient Threads::Threads)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
