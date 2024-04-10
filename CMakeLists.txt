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

set(SOURCES
        ${PROJECT_SOURCE_DIR}/src/findfiles.cpp
        ${PROJECT_SOURCE_DIR}/src/log.cpp
        ${PROJECT_SOURCE_DIR}/src/main.cpp
        ${PROJECT_SOURCE_DIR}/src/md5.cpp
        ${PROJECT_SOURCE_DIR}/src/module_config_reader.cpp
        ${PROJECT_SOURCE_DIR}/src/module_csv_reader.cpp
        ${PROJECT_SOURCE_DIR}/src/module_csv_writer.cpp
        ${PROJECT_SOURCE_DIR}/src/module_dbc_writer.cpp
        ${PROJECT_SOURCE_DIR}/src/module_parser.cpp
        ${PROJECT_SOURCE_DIR}/src/module_sql_writer.cpp
        ${PROJECT_SOURCE_DIR}/src/pch.cpp
        ${PROJECT_SOURCE_DIR}/src/ProgressBar.cpp
        ${PROJECT_SOURCE_DIR}/src/tinyxml2.cpp
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
