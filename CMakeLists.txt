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

# Allow tests to be run on this project
enable_testing()

# Add source code in subdirectories
add_subdirectory(cmake)
add_subdirectory(src)
add_subdirectory(modules)

# Install documentation
install(FILES README.md CHANGELOG.md
        DESTINATION ${INSTALL_DOC_DIR}
        COMPONENT doc
)


