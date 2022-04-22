set(CMAKE_C_FLAGS "-std=gnu99")

message("*************************build type " ${CMAKE_BUILD_TYPE} "*************************")

if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2 -finline-limit=10000 -DNDEBUG")
    set(CMAKE_C_FLAGS_RELEASE "$ENV{CFLAGS} -O2 -finline-limit=10000 -DNDEBUG")
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "Develop" OR
   "${CMAKE_BUILD_TYPE}" STREQUAL "UnitTest")
    set(CMAKE_CXX_FLAGS_UNITTEST "$ENV{CXXFLAGS} -O0 -ggdb -DDEBUG -DLIBEVDB_UNIT_TEST")
    set(CMAKE_C_FLAGS_UNITTEST "$ENV{CFLAGS} -O0 -ggdb -DDEBUG -DLIBEVDB_UNIT_TEST")
    set(CMAKE_*_FLAGS_UNITTEST "$ENV{CFLAGS} -O0 -ggdb -DDEBUG") # 链接文件编译时也支持gdb
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "Coverage")
    set(CMAKE_CXX_FLAGS_COVERAGE "$ENV{CXXFLAGS} -O0 -ggdb -DDEBUG -DLIBEVDB_UNIT_TEST --coverage")
    set(CMAKE_C_FLAGS_COVERAGE "$ENV{CFLAGS} -O0 -ggdb -DDEBUG -DLIBEVDB_UNIT_TEST --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

add_compile_definitions (LOG_USE_COLOR)

if("${CMAKE_BUILD_TYPE}" STREQUAL "UnitTest" OR
   "${CMAKE_BUILD_TYPE}" STREQUAL "Coverage")
    add_subdirectory (unittest)
endif()

add_subdirectory (lib)
add_subdirectory (third/ctest)
add_subdirectory (third/logc)
add_subdirectory (dbclient)
add_subdirectory (dbserver)
add_subdirectory (dbtest)
