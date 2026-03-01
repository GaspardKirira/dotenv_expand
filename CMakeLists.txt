cmake_minimum_required(VERSION 3.20)
project(dotenv_expand LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(dotenv_expand INTERFACE)
add_library(dotenv_expand::dotenv_expand ALIAS dotenv_expand)

target_include_directories(dotenv_expand INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

if (MSVC)
  target_compile_options(dotenv_expand INTERFACE /W4 /permissive-)
else()
  target_compile_options(dotenv_expand INTERFACE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
enable_testing()

add_executable(dotenv_expand_basic_test tests/test_basic.cpp)
target_link_libraries(dotenv_expand_basic_test PRIVATE dotenv_expand::dotenv_expand)
add_test(NAME dotenv_expand.basic COMMAND dotenv_expand_basic_test)
