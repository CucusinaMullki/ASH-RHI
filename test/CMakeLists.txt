find_package(glfw3 REQUIRED)
find_program(GLSLC_EXECUTABLE glslc REQUIRED)

file(GLOB_RECURSE ASH_TEST_SHADER_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.comp
)

set(ASH_TEST_SHADER_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${ASH_TEST_SHADER_OUTPUT_DIR})

set(ASH_TEST_COMPILED_SHADERS "")
foreach(SHADER ${ASH_TEST_SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SPIRV ${ASH_TEST_SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv)
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${GLSLC_EXECUTABLE} ${SHADER} -o ${SPIRV}
        DEPENDS ${SHADER}
        COMMENT "Compiling shader ${SHADER_NAME}"
        VERBATIM
    )
    list(APPEND ASH_TEST_COMPILED_SHADERS ${SPIRV})
endforeach()

add_custom_target(ash_test_shaders DEPENDS ${ASH_TEST_COMPILED_SHADERS})

add_executable(ash_test main.cpp)
add_dependencies(ash_test ash_test_shaders)

target_compile_definitions(ash_test PRIVATE "SHADER_DIR=\"${ASH_TEST_SHADER_OUTPUT_DIR}/\"")

target_link_libraries(ash_test PRIVATE
    $<LINK_LIBRARY:WHOLE_ARCHIVE,rhi>
    glfw
)

target_include_directories(ash_test PRIVATE ${CMAKE_SOURCE_DIR}/src)