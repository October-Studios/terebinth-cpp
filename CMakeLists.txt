cmake_minimum_required(VERSION 3.16)

project(terebinth)

set(terebinth_sources
    ${PROJECT_SOURCE_DIR}/src/terebinth/all_operators.cpp
    ${PROJECT_SOURCE_DIR}/src/terebinth/ast_node.cpp
    ${PROJECT_SOURCE_DIR}/src/terebinth/cpp_program.cpp
    ${PROJECT_SOURCE_DIR}/src/terebinth/error_handler.cpp
    ${PROJECT_SOURCE_DIR}/src/terebinth/lexer.cpp
    ${PROJECT_SOURCE_DIR}/src/terebinth/namespace.cpp
    ${PROJECT_SOURCE_DIR}/src/terebinth/parser.cpp
    ${PROJECT_SOURCE_DIR}/src/terebinth/resolve_literal.cpp
    ${PROJECT_SOURCE_DIR}/src/terebinth/source_file.cpp
    ${PROJECT_SOURCE_DIR}/src/terebinth/stack_frame.cpp
    ${PROJECT_SOURCE_DIR}/src/terebinth/terebinth_program.cpp
    ${PROJECT_SOURCE_DIR}/src/terebinth/terebinth_stdlib.cpp
    ${PROJECT_SOURCE_DIR}/src/terebinth/token.cpp
    ${PROJECT_SOURCE_DIR}/src/terebinth/type.cpp
    ${PROJECT_SOURCE_DIR}/util/file_utils.cpp
    ${PROJECT_SOURCE_DIR}/util/string_array.cpp
    ${PROJECT_SOURCE_DIR}/util/string_drawing.cpp
    ${PROJECT_SOURCE_DIR}/util/string_num_conversion.cpp
    ${PROJECT_SOURCE_DIR}/util/string_utils.cpp
)

add_library(terebinth_library SHARED ${terebinth_sources})

target_include_directories(terebinth_library PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(terebinth ${PROJECT_SOURCE_DIR}/src/terebinth/main.cpp)

target_link_libraries(terebinth PRIVATE terebinth_library)

set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR})

install(TARGETS terebinth DESTINATION bin)
install(TARGETS terebinth_library LIBRARY DESTINATION lib)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include DESTINATION include)
