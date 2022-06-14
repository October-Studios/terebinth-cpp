cmake_minimum_required(VERSION 3.5)

project(terebinth)

add_executable(${PROJECT_NAME} terebinth_sources)

target_include_directories(terebinth_headers PRIVATE ${PROJECT_SOURCE_DIR}/include)