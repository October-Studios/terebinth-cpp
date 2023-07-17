cmake_minimum_required(VERSION 3.26)

project(terebinth VERSION 0.1.0 LANGUAGES CXX)

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(src)
