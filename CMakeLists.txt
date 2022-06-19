cmake_minimum_required(VERSION 3.5)

project(terebinth)

set(terebinth_sources
    ${PROJECT_SOURCE_DIR}/src/terebinth/all_operators.cpp
    ${PROJECT_SOURCE_DIR}/src/terebinth/ast_node.cpp
    ${PROJECT_SOURCE_DIR}/src/terebinth/cpp_program.cpp
    ${PROJECT_SOURCE_DIR}/src/terebinth/error_handler.cpp
    ${PROJECT_SOURCE_DIR}/src/terebinth/lexer.cpp
    ${PROJECT_SOURCE_DIR}/src/terebinth/main.cpp
    ${PROJECT_SOURCE_DIR}/src/terebinth/namespace.cpp
    ${PROJECT_SOURCE_DIR}/src/terebinth/parser.cpp
    ${PROJECT_SOURCE_DIR}/src/terebinth/resolve_literal.cpp
    ${PROJECT_SOURCE_DIR}/src/terebinth/source_file.cpp
    ${PROJECT_SOURCE_DIR}/src/terebinth/stack_frame.cpp
    ${PROJECT_SOURCE_DIR}/src/terebinth/terebinth_program.cpp
    ${PROJECT_SOURCE_DIR}/src/terebinth/terebinth_stdlib.cpp
    ${PROJECT_SOURCE_DIR}/src/terebinth/token.cpp
    ${PROJECT_SOURCE_DIR}/src/terebinth/type.cpp
)

add_executable(terebinth_compiler ${terebinth_sources})

target_include_directories(terebinth_compiler PRIVATE ${PROJECT_SOURCE_DIR}/include)

set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR})

install(TARGETS terebinth_compiler DESTINATION bin)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include DESTINATION include)