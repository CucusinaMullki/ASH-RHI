file(GLOB_RECURSE RHI_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/*.cpp
)
file(GLOB_RECURSE RHI_HEADERS CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/include/*.hpp
    ${PROJECT_SOURCE_DIR}/include/*.h
)

if(RHI_BUILD_SHARED)
    add_library(rhi SHARED ${RHI_SOURCES} ${RHI_HEADERS})
else()
    add_library(rhi STATIC ${RHI_SOURCES} ${RHI_HEADERS})
endif()


target_include_directories(rhi
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(rhi PUBLIC Vulkan::Vulkan)

if(RHI_ENABLE_VALIDATION)
    target_compile_definitions(rhi PRIVATE
        $<$<CONFIG:Debug>:RHI_ENABLE_VALIDATION_LAYERS>
    )
endif()

include(GNUInstallDirs)

install(TARGETS rhi
    EXPORT AshTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT AshTargets
    FILE AshTargets.cmake
    NAMESPACE ASH::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ASH
)

target_compile_options(rhi PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>