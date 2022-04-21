add_definitions(-DEV_MULTIPLICITY)

aux_source_directory(. SRC_LIST)

add_executable(dbtest ${SRC_LIST})

link_directories (
    ${CMAKE_BINARY_DIR}/src/lib
)

target_link_libraries(dbtest
    -lev
    lib
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(dbtest Threads::Threads)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
