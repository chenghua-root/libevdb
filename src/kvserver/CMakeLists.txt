# INCLUDE_DIRECTORIES(/usr/local/include/)
# LINK_DIRECTORIES(/usr/local/lib/)

# 添加编译选项宏
# add_definitions(-DEV_MULTIPLICITY)

# 支持gdb
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

# 查找当前目录下的所有源文件
aux_source_directory(. SRC_LIST)

# 添加子目录
# add_subdirectory(lib)

link_directories (
    ${CMAKE_BINARY_DIR}/src/lib
)

# 指定生成目标
add_executable(libevkv ${SRC_LIST})
set_property(TARGET libevkv PROPERTY C_STANDARD 99)

# 添加链接库
target_link_libraries(libevkv lib)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(libevkv Threads::Threads)

target_link_libraries(libevkv -lev)

# 指定生成路径
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
