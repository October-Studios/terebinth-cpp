cmake_minimum_required(VERSION 3.27)

project(terebinth
  VERSION 0.1.0
  DESCRIPTION "Interpreted programming language built on C++20"
  LANGUAGES CXX
)

include(${CMAKE_SOURCE_DIR}/cmake/terebinth-api.cmake)

terebinth_project_defaults()

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
target_sources(terebinth_lib 
  PUBLIC 
  FILE_SET terebinth_modules 
  TYPE CXX_MODULES 
  FILES ${MODULE_FILES} build/modules/Version.ixx)

add_executable(terebinth apps/main.cpp)
target_link_libraries(terebinth PRIVATE terebinth_lib)

install(TARGETS terebinth EXPORT terebinth-targets
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(TARGETS terebinth EXPORT terebinth-targets
  DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_subdirectory(cmake)

install(FILES README.md CHANGELOG.md
        DESTINATION ${CMAKE_INSTALL_DOCDIR}
        COMPONENT doc
)
