project(nbt_dump CXX)
cmake_minimum_required(VERSION 3.18)

add_executable(nbt_dump src/nbt_dump.cpp)
add_library(nbt STATIC src/nbt.cpp)
target_include_directories(nbt PUBLIC include)
target_link_libraries(nbt_dump PRIVATE nbt)

add_executable(test_nbt
    test/test_main.cpp
    test/test_nbt.cpp
    test/test_swaps.cpp
)
find_package(Catch2 2 REQUIRED)
target_link_libraries(test_nbt PRIVATE Catch2::Catch2 nbt)
