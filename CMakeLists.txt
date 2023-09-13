cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)

project(
    Shortcut
    LANGUAGES CXX
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -O3")

include_directories(include)

set(SOURCES src/main.cpp src/utilities.cpp)

add_executable(Shortcut ${SOURCES})

add_custom_command(
  OUTPUT ${PROJECT_BINARY_DIR}/main.s
  DEPENDS src/main.cpp
  COMMAND ${CMAKE_CXX_COMPILER} -Iinclude -march=native -O3 -S -masm=intel src/main.cpp -o build/main.s
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

add_custom_target(Assembly ALL
  DEPENDS ${PROJECT_BINARY_DIR}/main.s
)

