cmake_minimum_required (VERSION 2.8)

if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set (CMAKE_CXX_FLAGS "--std=c++14 ${CMAKE_CXX_FLAGS}")
    endif ()
else ()
    set (CMAKE_CXX_STANDARD 14)
endif ()

include_directories(sources/)

file(GLOB_RECURSE SOURCES sources/*.cpp *.h)

add_library(fsal STATIC ${SOURCES})

if(MSVC)
else()
    target_compile_options(fsal PRIVATE -lstdc++fs -Wall -Wno-switch)
endif()
