cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_COMPILER g++)

project(wowparser VERSION 4.0
        LANGUAGES CXX )

# Opciones para seleccionar la plataforma
option(BUILD_LINUX32 "Build for Linux 32-bit" OFF)
option(BUILD_LINUX64 "Build for Linux 64-bit" OFF)
option(BUILD_WIN32 "Build for Windows 32-bit" OFF)
option(BUILD_WIN64 "Build for Windows 64-bit" OFF)
option(BUILD_ALL_PLATFORMS "Build for all platforms" ON)

# Configuración de las opciones de compilación
if (BUILD_ALL_PLATFORMS)
    set(BUILD_LINUX32 ON)
    set(BUILD_LINUX64 ON)
    set(BUILD_WIN32 ON)
    set(BUILD_WIN64 ON)
endif()

if (NOT BUILD_LINUX32 AND NOT BUILD_LINUX64 AND NOT BUILD_WIN32 AND NOT BUILD_WIN64)
    message(FATAL_ERROR "Por favor, selecciona al menos una plataforma para compilar.")
endif()

set(CMAKE_CXX_FLAGS "-Os -s -Wall -msse2 -mfpmath=sse -std=c++20 -static -static-libgcc -static-libstdc++")

file(GLOB source_parser ${PROJECT_SOURCE_DIR}/src/**.cpp ${PROJECT_SOURCE_DIR}/src/csv/**.cpp)
file(GLOB source_lib ${PROJECT_SOURCE_DIR}/lib/tinyxml2/**.cpp ${PROJECT_SOURCE_DIR}/lib/md5/**.cpp)

set(SOURCES
        ${source_parser}
        ${source_lib}
)

execute_process(COMMAND bash ${PROJECT_SOURCE_DIR}/revision.sh)

if (BUILD_LINUX32)
    add_subdirectory(platform/linux32)
endif()

if (BUILD_LINUX64)
    add_subdirectory(platform/linux64)
endif()

if (BUILD_WIN32)
    add_subdirectory(platform/win32)
endif()

if (BUILD_WIN64)
    add_subdirectory(platform/win64)
endif()
