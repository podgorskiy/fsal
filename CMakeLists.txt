cmake_minimum_required (VERSION 2.8)

if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set (CMAKE_CXX_FLAGS "--std=c++14 ${CMAKE_CXX_FLAGS}")
    endif ()
else ()
    set (CMAKE_CXX_STANDARD 14)
endif ()

find_package(doctest REQUIRED)

include_directories(sources/)

file(GLOB_RECURSE SOURCES sources/*.cpp *.h)

add_library(fsal STATIC ${SOURCES})

add_executable(tests tests/main.cpp)
target_link_libraries(tests PRIVATE fsal stdc++fs z doctest::doctest)

if(MSVC)
else()
    target_compile_options(fsal PRIVATE -lstdc++fs-Wall -Wno-switch)
    target_compile_options(tests PRIVATE -lstdc++fs -Wall -Wno-switch)
endif()
