cmake_minimum_required(VERSION 3.14)
project(nv_export LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Release)

option(DRAM_TELEMETRY "Disable DRAM telemetry and don't link pci library" ON)

include(cmake/CPM.cmake)

CPMAddPackage("gh:ericniebler/range-v3#0.12.0")
CPMAddPackage("gh:boost-ext/reflect#v1.1.1")

add_library(reflect INTERFACE)
target_include_directories(reflect SYSTEM INTERFACE ${reflect_SOURCE_DIR})
add_library(reflect::reflect ALIAS reflect)


find_package(CUDAToolkit REQUIRED)

add_executable(${PROJECT_NAME} nv_export.cpp)

if (DRAM_TELEMETRY)
  message(STATUS "Building with DRAM telemetry")
  find_library(PCI_LIBRARY NAMES pci)
  find_path(PCI_INCLUDE_DIR NAMES pci/pci.h)
  target_link_libraries(${PROJECT_NAME} PRIVATE ${PCI_LIBRARY})
  target_include_directories(${PROJECT_NAME} PRIVATE ${PCI_INCLUDE_DIR})
else()
  message(STATUS "Building without DRAM telemetry")
  target_compile_definitions(${PROJECT_NAME} PRIVATE NO_DRAM_TELEMETRY)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE 
    CUDA::nvml
    reflect::reflect
    range-v3::range-v3
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CUDAToolkit_INCLUDE_DIRS}
)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
