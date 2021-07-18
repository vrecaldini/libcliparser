cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

project(cliparser)
include_directories(./)
add_library(cliparser STATIC libcliparser/cliparser.cpp)

add_executable(test test/test.cpp)  # test
target_link_libraries(test PUBLIC cliparser)

add_executable(checkpath example/checkpath.cpp)  # example
target_link_libraries(checkpath PUBLIC cliparser)


if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /w /DNDEBUG") 
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -w -DNDEBUG")

endif()


