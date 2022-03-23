set(DIR_UNITTEST_SRCS
    lib/s3_connection_test.c
    main_test.c
)

link_directories (
    ${CMAKE_BINARY_DIR}/src
)

add_executable(libevkv_test ${DIR_UNITTEST_SRCS})

# 添加链接库
target_link_libraries(libevkv_test
    -lev
    -lprotobuf-c
    ctest
    lib
)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(libevkv_test Threads::Threads)

# 指定生成路径
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin-unittest)
