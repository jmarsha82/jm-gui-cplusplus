cmake_minimum_required(VERSION 3.20)

project(JmGuiCppApps LANGUAGES CXX)

add_library(CgpaCore INTERFACE)
target_include_directories(CgpaCore INTERFACE src)
target_compile_features(CgpaCore INTERFACE cxx_std_17)

add_executable(TaskListApp WIN32
    src/main.cpp
)

target_compile_features(TaskListApp PRIVATE cxx_std_17)

target_link_libraries(TaskListApp PRIVATE
    comctl32
)

add_executable(CGPACalculator WIN32
    src/cgpa_calculator.cpp
)

target_compile_features(CGPACalculator PRIVATE cxx_std_17)

target_link_libraries(CGPACalculator PRIVATE
    CgpaCore
    comctl32
)

add_executable(CgpaCoreTests
    tests/cgpa_core_tests.cpp
)

target_link_libraries(CgpaCoreTests PRIVATE CgpaCore)

enable_testing()
add_test(NAME CgpaCoreTests COMMAND CgpaCoreTests)
