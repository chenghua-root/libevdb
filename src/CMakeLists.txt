# INCLUDE_DIRECTORIES(/usr/local/include/)
# LINK_DIRECTORIES(/usr/local/lib/)

# 查找当前目录下的所有源文件
aux_source_directory(. SRC_LIST)

# 添加子目录
# add_subdirectory(lib)

# 指定生成目标
add_executable(libevkv ${SRC_LIST})

# 添加链接库
# target_link_libraries(libevkv Lib)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(libevkv Threads::Threads)

target_link_libraries(libevkv -lev)

# 指定生成路径
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
