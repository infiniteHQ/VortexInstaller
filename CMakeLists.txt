cmake_minimum_required(VERSION 3.16)
project(vortex_installer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(BUNDLE_EXECUTABLE true)

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ui/installer ui_installer_build)
if (NOT TARGET httpcl)
add_subdirectory(lib/httpcl)
endif()

add_executable(vortex_installer src/installer.cpp)
target_link_libraries(vortex_installer PUBLIC installer httpcl spdlog cherry)

if (WIN32)
    target_link_libraries(vortex_installer PUBLIC urlmon)
endif()
if(UNIX AND NOT APPLE)
    find_package(CURL REQUIRED)
    target_link_libraries(vortex_installer PRIVATE CURL::libcurl)
endif()

add_executable(vortex_update src/updater.cpp)
target_link_libraries(vortex_update PUBLIC installer httpcl spdlog cherry)
if (WIN32)
    target_link_libraries(vortex_update PUBLIC urlmon)
endif()
if(UNIX AND NOT APPLE)
    find_package(CURL REQUIRED)
    target_link_libraries(vortex_update PRIVATE CURL::libcurl)
endif()

add_executable(vortex_uninstall src/uninstall.cpp)
target_link_libraries(vortex_uninstall PUBLIC installer httpcl spdlog cherry)
if (WIN32)
    target_link_libraries(vortex_uninstall PUBLIC urlmon)
endif()
if(UNIX AND NOT APPLE)
    find_package(CURL REQUIRED)
    target_link_libraries(vortex_uninstall PRIVATE CURL::libcurl)
endif()


add_executable(vxinstaller src/vxinstaller.cpp)
target_link_libraries(vxinstaller PUBLIC installer httpcl spdlog cherry)
if (WIN32)
    target_link_libraries(vxinstaller PUBLIC urlmon)
endif()
if(UNIX AND NOT APPLE)
    find_package(CURL REQUIRED)
    target_link_libraries(vxinstaller PRIVATE CURL::libcurl)
endif()

add_executable(vxuninstall src/vxuninstall.cpp)
target_link_libraries(vxuninstall PUBLIC installer httpcl spdlog cherry)
if (WIN32)
    target_link_libraries(vxuninstall PUBLIC urlmon)
endif()
if(UNIX AND NOT APPLE)
    find_package(CURL REQUIRED)
    target_link_libraries(vxuninstall PRIVATE CURL::libcurl)
endif()

if(WIN32)
    set_target_properties(vortex_installer PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

if(WIN32)
    set_target_properties(vortex_update PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
if(WIN32)
    set_target_properties(vortex_uninstall PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
if(WIN32)
    set_target_properties(vxinstaller PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
if(WIN32)
    set_target_properties(vxuninstall PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()


