cmake_minimum_required(VERSION 3.27)

set(TEREBINTH_VERSION_MAJOR 0)
set(TEREBINTH_VERSION_MINOR 1)
set(TEREBINTH_VERSION_PATCH 0)
set(TEREBINTH_VERSION "${TEREBINTH_VERSION_MAJOR}.${TEREBINTH_VERSION_MINOR}.${TEREBINTH_VERSION_PATCH}")

project(terebinth
  VERSION ${TEREBINTH_VERSION}
  DESCRIPTION "Interpreted programming language built on C++20"
  LANGUAGES CXX
)

set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "aa1f7df0-828a-4fcd-9afc-2dc80491aca7")
string(CONCAT CMAKE_EXPERIMENTAL_CXX_SCANDEP_SOURCE
  "<CMAKE_CXX_COMPILER> <DEFINES> <INCLUDES> <FLAGS> <SOURCE>"
  " -MT <DYNDEP_FILE> -MD -MF <DEP_FILE>"
  " ${flags_to_scan_deps} -fdep-file=<DYNDEP_FILE> -fdep-output=<OBJECT>"
  )

set(CMAKE_EXPERIMENTAL_CXX_MODULE_MAP_FORMAT "gcc")
set(CMAKE_EXPERIMENTAL_CXX_MODULE_MAP_FLAG
  "${compiler_flags_for_module_map} -fmodule-mapper=<MODULE_MAP_FILE>")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

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
