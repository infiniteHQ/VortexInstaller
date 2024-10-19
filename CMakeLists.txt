cmake_minimum_required(VERSION 3.16)
project(vortex_installer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(VORTEX_VERSION "1.1")
set(vortex_installer_SHARED_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")


add_library(vortex_installer_shared SHARED ${vortex_installer_SHARED_SOURCES})

target_include_directories(vortex_installer_shared PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/main/include
    ${CMAKE_CURRENT_SOURCE_DIR}/main/src
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ui/installer ui_installer_build)
target_link_libraries(vortex_installer_shared PUBLIC installer)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/restcpp restcpp_build)
target_link_libraries(vortex_installer_shared PUBLIC restclient-cpp)

target_link_libraries(vortex_installer_shared PUBLIC spdlog)


add_executable(vortex_installer ${VORTEX_SOURCES} main.cpp)
target_link_libraries(vortex_installer PRIVATE vortex_installer_shared)
