cmake_minimum_required(VERSION 3.14)
project(nv_export LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/CPM.cmake)

CPMAddPackage(
  Name reflect
  GITHUB_REPOSITORY boost-ext/reflect
  GIT_TAG v1.1.1
)
add_library(reflect INTERFACE)
target_include_directories(reflect SYSTEM INTERFACE ${reflect_SOURCE_DIR})
add_library(reflect::reflect ALIAS reflect)

find_package(CUDAToolkit REQUIRED)

add_executable(${PROJECT_NAME} nv_export.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    CUDA::nvml
    reflect::reflect
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CUDAToolkit_INCLUDE_DIRS}
)