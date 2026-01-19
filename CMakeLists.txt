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

if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fdata-sections -ffunction-sections -D_GLIBCXX_USE_CXX11_ABI=1")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections -s")
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ui/installer ui_installer_build)
if (NOT TARGET httpcl)
add_subdirectory(lib/httpcl)
endif()
add_executable(vortex_installer src/installer.cpp src/base.cpp)
target_link_libraries(vortex_installer PRIVATE installer httpcl cherry)

if (WIN32)
    target_link_libraries(vortex_installer PRIVATE urlmon)
endif()
if(UNIX AND NOT APPLE)
endif()

add_executable(vortex_update src/updater.cpp src/base.cpp)
target_link_libraries(vortex_update PRIVATE installer httpcl cherry)
if (WIN32)
    target_link_libraries(vortex_update PRIVATE urlmon)
endif()
if(UNIX AND NOT APPLE)
endif()

add_executable(vortex_uninstall src/uninstall.cpp)
target_link_libraries(vortex_uninstall PRIVATE installer httpcl cherry)
if (WIN32)
    target_link_libraries(vortex_uninstall PRIVATE urlmon)
endif()
if(UNIX AND NOT APPLE)
endif()


add_executable(vxinstaller src/vxinstaller.cpp src/base.cpp)
target_link_libraries(vxinstaller PRIVATE installer httpcl cherry)
if (WIN32)
    target_link_libraries(vxinstaller PRIVATE urlmon)
endif()
if(UNIX AND NOT APPLE)
endif()

add_executable(vxuninstall src/vxuninstall.cpp)
target_link_libraries(vxuninstall PRIVATE installer httpcl cherry)
if (WIN32)
    target_link_libraries(vxuninstall PRIVATE urlmon)
endif()
if(UNIX AND NOT APPLE)
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


