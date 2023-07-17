add_subdirectory(actions)
add_subdirectory(util)

configure_file(Version.h.in Version.h @ONLY)

set(terebinth_src
    ${CMAKE_CURRENT_SOURCE_DIR}/all_operators.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ast_node.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp_program.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/error_handler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/namespace.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/resolve_literal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source_file.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/stack_frame.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/terebinth_program.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/token.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/type.cpp
)

add_executable(terebinth ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

install(TARGETS terebinth DESTINATION ${INSTALL_BIN_DIR})