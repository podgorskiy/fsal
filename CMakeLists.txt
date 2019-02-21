cmake_minimum_required (VERSION 2.8)

include_directories(sources/)

file(GLOB_RECURSE SOURCES sources/*.cpp *.h)

add_library(fsal STATIC ${SOURCES})
