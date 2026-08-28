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
find_program(GLSLC_EXECUTABLE glslc REQUIRED)

file(GLOB_RECURSE RHI_SHADER_SOURCES
    ${CMAKE_SOURCE_DIR}/shaders/*.vert
    ${CMAKE_SOURCE_DIR}/shaders/*.frag
    ${CMAKE_SOURCE_DIR}/shaders/*.comp
    ${CMAKE_SOURCE_DIR}/shaders/*.geom
    ${CMAKE_SOURCE_DIR}/shaders/*.tesc
    ${CMAKE_SOURCE_DIR}/shaders/*.tese
)

set(RHI_SHADER_OUTPUT_DIR ${CMAKE_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${RHI_SHADER_OUTPUT_DIR})

set(RHI_COMPILED_SHADERS "")
foreach(SHADER ${RHI_SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SPIRV ${RHI_SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv)
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${GLSLC_EXECUTABLE} ${SHADER} -o ${SPIRV}
        DEPENDS ${SHADER}
        COMMENT "Shader compilation ${SHADER_NAME}"
        VERBATIM
    )
    list(APPEND RHI_COMPILED_SHADERS ${SPIRV})
endforeach()

add_custom_target(rhi_shaders DEPENDS ${RHI_COMPILED_SHADERS})

add_subdirectory(src)

if(ASH_BUILD_TEST)
    add_subdirectory(test)
endif()

if(RHI_BUILD_EXAMPLES AND EXISTS ${CMAKE_SOURCE_DIR}/examples)
    add_subdirectory(examples)
endif()