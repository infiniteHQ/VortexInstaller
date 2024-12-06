cmake_minimum_required(VERSION 3.16)
project(vortex_installer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(VORTEX_VERSION "1.1")
set(BUNDLE_EXECUTABLE true)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ui/installer ui_installer_build)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/restcpp restcpp_build)

add_executable(vortex_installer src/installer.cpp)
target_link_libraries(vortex_installer PUBLIC installer restclient-cpp spdlog cherry)

if (WIN32)
    target_link_libraries(vortex_installer PUBLIC urlmon)
endif()

add_executable(vortex_update src/updater.cpp)
target_link_libraries(vortex_update PUBLIC installer restclient-cpp spdlog cherry)
if (WIN32)
    target_link_libraries(vortex_update PUBLIC urlmon)
endif()

add_executable(vortex_uninstall src/uninstall.cpp)
target_link_libraries(vortex_uninstall PUBLIC installer restclient-cpp spdlog cherry)
if (WIN32)
    target_link_libraries(vortex_uninstall PUBLIC urlmon)
endif()

add_executable(vxinstaller src/vxinstaller.cpp)
target_link_libraries(vxinstaller PUBLIC installer restclient-cpp spdlog cherry)
if (WIN32)
    target_link_libraries(vxinstaller PUBLIC urlmon)
endif()

add_executable(vxuninstall src/vxuninstall.cpp)
target_link_libraries(vxuninstall PUBLIC installer restclient-cpp spdlog cherry)
if (WIN32)
    target_link_libraries(vxuninstall PUBLIC urlmon)
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


