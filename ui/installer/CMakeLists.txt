cmake_minimum_required(VERSION 3.16)
project(installer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Build options
option(CHERRY_STATIC "Link Cherry statically" ON)
option(CHERRY_DEBUG "Enable Cherry debug tools" ON)
option(CHERRY_PACKAGE "Enable packaging features with pyinstaller" OFF)
option(CHERRY_ENABLE_NET "Enable net features" ON)
option(CHERRY_ENABLE_AUDIO "Enable audio features" OFF)
option(CHERRY_ENABLE_LINUXDRM "Enable Linux Direct Rendering Manager" OFF)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../lib/cherry cherry_build)

file(
    GLOB_RECURSE
    UI_installer
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h" 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

message(STATUS "UI_installer sources: ${UI_installer}")

if (UI_installer)
    add_library(installer STATIC ${UI_installer})
else()
    message(FATAL_ERROR "No source files found for the installer target.")
endif()

#if(WIN32)
#    set_target_properties(installer PROPERTIES
#        WIN32_EXECUTABLE TRUE
#    )
#endif()

target_link_libraries(installer PRIVATE cherry)

add_custom_command(TARGET installer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
)

