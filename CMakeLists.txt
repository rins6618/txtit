cmake_minimum_required(VERSION 3.28)
project(txtit VERSION 0.11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)

    endif()
    
add_executable(txtit src/txtit.cpp src/editor.cpp src/platform_win32.cpp src/platform_posix.cpp)