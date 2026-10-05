cmake_minimum_required(VERSION 3.16)
project(vortex_installer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fPIC")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(BUNDLE_EXECUTABLE true)

find_package(Vulkan REQUIRED)
if(APPLE)
    find_package(OpenSSL REQUIRED)
    find_package(Threads REQUIRED)
endif()

if(APPLE)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    set(CMAKE_THREAD_LIBS_INIT "-pthread")
    set(CMAKE_HAVE_THREADS_LIBRARY 1)
    set(CMAKE_USE_PTHREADS_INIT 1)
endif()

include_directories(${Vulkan_INCLUDE_DIRS})

if(UNIX)
    find_package(CURL REQUIRED)
    find_package(OpenSSL REQUIRED)
endif()

if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fdata-sections -ffunction-sections")

    if(NOT APPLE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=1")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections -s")
    endif()
else()

    add_compile_options(/O1 /GL /Gw /Gy)
    add_link_options(/OPT:REF /OPT:ICF /LTCG)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/ui ui_installer_build)
if (NOT TARGET httpcl)
    add_subdirectory(lib/httpcl)
endif()

# --- vortex_installer_backend ---
add_executable(vortex_installer_backend
  src/backend.cpp
  src/api/api.cpp
)

target_link_libraries(vortex_installer_backend PRIVATE httpcl)
if(APPLE)
    target_link_libraries(vortex_installer_backend PRIVATE objc)
    target_link_libraries(vortex_installer_backend PRIVATE Threads::Threads)
endif()

if (WIN32)
    target_link_libraries(vortex_installer_backend PRIVATE urlmon winhttp)
endif()
if(UNIX)
    target_link_libraries(vortex_installer_backend PRIVATE CURL::libcurl
    OpenSSL::Crypto)
endif()

# --- vortex_installer ---
add_executable(vortex_installer src/installer.cpp src/api/api.cpp)
target_link_libraries(vortex_installer PRIVATE installer httpcl cherry)
if(APPLE)
    target_link_libraries(vortex_installer PRIVATE Threads::Threads)
endif()
if (WIN32)
    target_link_libraries(vortex_installer PRIVATE urlmon winhttp)
endif()
if(UNIX)
    target_link_libraries(vortex_installer PRIVATE CURL::libcurl
    OpenSSL::Crypto)
endif()

# --- vortex_update ---
add_executable(vortex_update src/updater.cpp src/api/api.cpp)
target_link_libraries(vortex_update PRIVATE installer httpcl cherry)
if(APPLE)
    target_link_libraries(vortex_update PRIVATE Threads::Threads)
endif()
if (WIN32)
    target_link_libraries(vortex_update PRIVATE urlmon winhttp)
endif()
if(UNIX)
    target_link_libraries(vortex_update PRIVATE CURL::libcurl
    OpenSSL::Crypto)
endif()

# --- vortex_uninstall ---
add_executable(vortex_uninstall src/uninstall.cpp src/api/api.cpp)
target_link_libraries(vortex_uninstall PRIVATE installer httpcl cherry)
if(APPLE)
    target_link_libraries(vortex_uninstall PRIVATE Threads::Threads)
endif()
if (WIN32)
    target_link_libraries(vortex_uninstall PRIVATE urlmon winhttp)
endif()
if(UNIX)
    target_link_libraries(vortex_uninstall PRIVATE CURL::libcurl
    OpenSSL::Crypto)
endif()

# --- vxinstaller ---
add_executable(vxinstaller src/vxinstaller.cpp src/api/api.cpp)
target_link_libraries(vxinstaller PRIVATE installer httpcl cherry)
if(APPLE)
    target_link_libraries(vxinstaller PRIVATE Threads::Threads)
endif()
if (WIN32)
    target_link_libraries(vxinstaller PRIVATE urlmon winhttp)
endif()
if(UNIX)
    target_link_libraries(vxinstaller PRIVATE CURL::libcurl
    OpenSSL::Crypto)
endif()

# --- vxuninstall ---
add_executable(vxuninstall src/vxuninstall.cpp src/api/api.cpp)
target_link_libraries(vxuninstall PRIVATE installer httpcl cherry)
if(APPLE)
    target_link_libraries(vxuninstall PRIVATE Threads::Threads)
endif()
if (WIN32)
    target_link_libraries(vxuninstall PRIVATE urlmon winhttp)
endif()
if(UNIX)
    target_link_libraries(vxuninstall PRIVATE CURL::libcurl
    OpenSSL::Crypto)
endif()

if(WIN32)
    set_target_properties(vortex_installer_backend PROPERTIES WIN32_EXECUTABLE TRUE)
    set_target_properties(vortex_installer PROPERTIES WIN32_EXECUTABLE TRUE)
    set_target_properties(vortex_update PROPERTIES WIN32_EXECUTABLE TRUE)
    set_target_properties(vortex_uninstall PROPERTIES WIN32_EXECUTABLE TRUE)
    set_target_properties(vxinstaller PROPERTIES WIN32_EXECUTABLE TRUE)
    set_target_properties(vxuninstall PROPERTIES WIN32_EXECUTABLE TRUE)
endif()