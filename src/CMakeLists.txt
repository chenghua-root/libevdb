set(CMAKE_C_FLAGS "-std=gnu99")

add_subdirectory (third/ctest)
add_subdirectory (lib)
add_subdirectory (kvserver)
add_subdirectory (kvclient)

#add_subdirectory (unittest/lib)
add_subdirectory (unittest)
