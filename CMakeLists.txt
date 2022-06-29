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

set(terebinth_headers
    ${PROJECT_SOURCE_DIR}/include/VERSION.h
    ${PROJECT_SOURCE_DIR}/include/action.h
    ${PROJECT_SOURCE_DIR}/include/all_operators.h
    ${PROJECT_SOURCE_DIR}/include/ast_node.h
    ${PROJECT_SOURCE_DIR}/include/cpp_program.h
    ${PROJECT_SOURCE_DIR}/include/error_handler.h
    ${PROJECT_SOURCE_DIR}/include/namespace.h
    ${PROJECT_SOURCE_DIR}/include/operator.h
    ${PROJECT_SOURCE_DIR}/include/source_file.h
    ${PROJECT_SOURCE_DIR}/include/stack_frame.h
    ${PROJECT_SOURCE_DIR}/include/terebinth_program.h
    ${PROJECT_SOURCE_DIR}/include/token.h
    ${PROJECT_SOURCE_DIR}/include/type.h
    ${PROJECT_SOURCE_DIR}/include/util/file_utils.h
    ${PROJECT_SOURCE_DIR}/include/util/string_array.h
    ${PROJECT_SOURCE_DIR}/include/util/string_drawing.h
    ${PROJECT_SOURCE_DIR}/include/util/string_num_conversion.h
    ${PROJECT_SOURCE_DIR}/include/util/string_utils.h
)

add_library(terebinth_library SHARED
    ${terebinth_headers} ${terebinth_sources}
)
add_executable(terebinth ${PROJECT_SOURCE_DIR}/src/terebinth/main.cpp)
target_link_libraries(terebinth PRIVATE terebinth_library)

set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR})

install(TARGETS terebinth DESTINATION bin)
install(TARGETS terebinth_library LIBRARY DESTINATION lib)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include DESTINATION include)
