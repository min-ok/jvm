cmake_minimum_required(VERSION 3.16)
project(vm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(vm_core INTERFACE)
target_include_directories(vm_core INTERFACE src)


add_library(vm_loader src/loader/loader.cpp)
target_link_libraries(vm_loader PUBLIC vm_core)


add_executable(vm src/main.cpp)
target_link_libraries(vm PRIVATE vm_core vm_loader)
