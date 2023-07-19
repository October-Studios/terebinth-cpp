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

set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "2182bf5c-ef0d-489a-91da-49dbc3090d2a")
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3 -fmodules-ts")

add_library(terebinth_lib)

file(GLOB_RECURSE SRC_FILES
  src/terebinth/*.cpp
  src/terebinth/actions/*.cpp
  src/terebinth/util/*.cpp
)
target_sources(terebinth_lib PUBLIC ${SRC_FILES})

configure_file(modules/Version.ixx.in modules/Version.ixx @ONLY)
file(GLOB_RECURSE MODULE_FILES
  modules/*.ixx
  modules/util/*.ixx
)
target_sources(terebinth_lib PUBLIC FILE_SET terebinth_modules TYPE CXX_MODULES FILES ${MODULE_FILES})

add_executable(terebinth apps/main.cpp)
target_link_libraries(terebinth PRIVATE terebinth_lib)
# Allow tests to be run on this project
enable_testing()
