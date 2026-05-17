cmake_minimum_required(VERSION 3.20)

project(TaskListApp LANGUAGES CXX)

add_executable(TaskListApp WIN32
    src/main.cpp
)

target_compile_features(TaskListApp PRIVATE cxx_std_17)

target_link_libraries(TaskListApp PRIVATE
    comctl32
)
