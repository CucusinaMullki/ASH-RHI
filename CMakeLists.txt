cmake_minimum_required(VERSION 3.20)
project(ASH LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(RHI_BUILD_SHARED      "Build RHI like shared-library" OFF)
option(RHI_ENABLE_VALIDATION "Turn ON Vulkan validation layers in Debug" ON)
option(RHI_BUILD_EXAMPLES    "Build examples/test apps" ON)
option(ASH_BUILD_TEST        "Build the test application (test/)" ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

find_package(Vulkan REQUIRED)

add_subdirectory(src)

if(ASH_BUILD_TEST)
    add_subdirectory(test)
endif()

if(RHI_BUILD_EXAMPLES AND EXISTS ${CMAKE_SOURCE_DIR}/examples)
    add_subdirectory(examples)
endif()