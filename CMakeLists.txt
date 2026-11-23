cmake_minimum_required(VERSION 3.16)
project(vm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Wpedantic)

add_library(vm_core INTERFACE)
target_include_directories(vm_core INTERFACE src)

add_library(vm_engine STATIC src/vm/vm.cpp)
target_link_libraries(vm_engine PUBLIC vm_core)

add_library(vm_loader STATIC src/loader/loader.cpp)
target_link_libraries(vm_loader PUBLIC vm_core)

add_executable(vm src/main.cpp)
target_link_libraries(vm PRIVATE vm_engine vm_loader)
