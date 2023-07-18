cmake_minimum_required(VERSION 3.26)

set(TEREBINTH_VERSION_MAJOR 0)
set(TEREBINTH_VERSION_MINOR 1)
set(TEREBINTH_VERSION_PATCH 0)
set(TEREBINTH_VERSION "${TEREBINTH_VERSION_MAJOR}.${TEREBINTH_VERSION_MINOR}.${TEREBINTH_VERSION_PATCH}")

project(terebinth
  VERSION ${TEREBINTH_VERSION}
  DESCRIPTION "Interpreted programming language built on C++20"
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3")

include_directories(modules)

configure_file(modules/Version.cppm.in modules/Version.cppm @ONLY)

file(GLOB_RECURSE SRC_FILES
  src/terebinth/*.cpp
  src/terebinth/actions/*.cpp
  src/terebinth/util/*.cpp
)

add_executable(terebinth ${SRC_FILES})

# Allow tests to be run on this project
enable_testing()
