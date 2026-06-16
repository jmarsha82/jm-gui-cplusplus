cmake_minimum_required(VERSION 3.20)

project(JmGuiCppApps LANGUAGES CXX)

include(FetchContent)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/6.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(raylib)

add_library(CgpaCore INTERFACE)
target_include_directories(CgpaCore INTERFACE src)
target_compile_features(CgpaCore INTERFACE cxx_std_17)

add_executable(TaskListApp
    src/main.cpp
)

target_compile_features(TaskListApp PRIVATE cxx_std_17)

target_link_libraries(TaskListApp PRIVATE
    raylib
)

add_executable(CGPACalculator
    src/cgpa_calculator.cpp
)

target_compile_features(CGPACalculator PRIVATE cxx_std_17)

target_link_libraries(CGPACalculator PRIVATE
    CgpaCore
    raylib
)

add_executable(SnakeGame
    src/snake_game.cpp
)

target_compile_features(SnakeGame PRIVATE cxx_std_17)

target_link_libraries(SnakeGame PRIVATE
    raylib
)

add_custom_target(SnakeQtGame DEPENDS SnakeGame)

add_executable(BreakBricksGame
    src/break_bricks_game.cpp
)

target_compile_features(BreakBricksGame PRIVATE cxx_std_17)

target_link_libraries(BreakBricksGame PRIVATE
    raylib
)

add_executable(CgpaCoreTests
    tests/cgpa_core_tests.cpp
)

target_link_libraries(CgpaCoreTests PRIVATE CgpaCore)

enable_testing()
add_test(NAME CgpaCoreTests COMMAND CgpaCoreTests)
