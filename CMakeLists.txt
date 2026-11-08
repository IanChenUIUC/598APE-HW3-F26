cmake_minimum_required(VERSION 4.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES src/main.cpp src/sandpile.cpp)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

project(sandpile)
add_executable(sandpile ${SOURCES})
target_compile_options(sandpile PRIVATE -march=native)

target_include_directories(sandpile PRIVATE ${PROJECT_SOURCE_DIR}/include)

find_package(OpenMP COMPONENTS CXX REQUIRED)
target_link_libraries(sandpile PRIVATE OpenMP::OpenMP_CXX)
