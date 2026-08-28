find_package(glfw3 REQUIRED)

add_executable(ash_test main.cpp)

target_link_libraries(ash_test PRIVATE
    $<LINK_LIBRARY:WHOLE_ARCHIVE,rhi>
    glfw
)

target_include_directories(ash_test PRIVATE ${CMAKE_SOURCE_DIR}/src)