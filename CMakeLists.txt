cmake_minimum_required(VERSION 3.30)
project(untitled3)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-DUNICODE -D_UNICODE)

file(GLOB_RECURSE SOURCE_FILES source/*.cpp source/*.h)

add_executable(untitled3 WIN32 main.cpp resource.h ${SOURCE_FILES})

target_include_directories(untitled3 PRIVATE source)

target_sources(untitled3 PRIVATE app.ico resources.rc resource.h app.manifest)