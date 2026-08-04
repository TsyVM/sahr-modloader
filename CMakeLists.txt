cmake_minimum_required(VERSION 3.25)
project(SHARModLoader VERSION 1.0.0 LANGUAGES C CXX)

# VanHooks requires C++23 (std::expected).
# MSVC 19.38+ (VS 2022 17.6+) is recommended.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# SHAR is a 32-bit game — force Win32.
if(MSVC)
    set(CMAKE_GENERATOR_PLATFORM Win32)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
endforeach()

# ── VanHooks drop-in SDK ──────────────────────────────────────────────────────
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(VH_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vanhooks/lib/win-x64")
else()
    set(VH_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vanhooks/lib/win-x86")
endif()

add_library(VanHooks::vanhooks STATIC IMPORTED)
set_target_properties(VanHooks::vanhooks PROPERTIES
    IMPORTED_LOCATION_RELEASE "${VH_LIB_DIR}/Release/vanhooks.lib"
    IMPORTED_LOCATION_DEBUG   "${VH_LIB_DIR}/Release/vanhooks.lib"
    IMPORTED_LOCATION         "${VH_LIB_DIR}/Release/vanhooks.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/vanhooks/include"
)

# ── SHARModLoader (.asi) ──────────────────────────────────────────────────────
# Output as .asi so ASI Ultimate Loader picks it up automatically.

add_library(SHARModLoader SHARED
    src/main.cpp
    src/mod_manager.cpp
    src/file_hooks.cpp
    src/utils.cpp
)

set_target_properties(SHARModLoader PROPERTIES
    SUFFIX ".asi"
)

target_include_directories(SHARModLoader PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(SHARModLoader PRIVATE
    VanHooks::vanhooks
    dbghelp
)

target_compile_definitions(SHARModLoader PRIVATE
    _CRT_SECURE_NO_WARNINGS
    UNICODE
    _UNICODE
)

# Match VanHooks' /MT static CRT to avoid CRT mismatch in the loaded module.
if(MSVC)
    set_property(TARGET SHARModLoader PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded")

    target_compile_options(SHARModLoader PRIVATE
        /W4
        /WX-
        /EHa
    )
else()
    target_compile_options(SHARModLoader PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Create placeholder mods directory in build output
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/mods)

message(STATUS "=== SHAR ModLoader ===")
message(STATUS "Output: ${CMAKE_BINARY_DIR}/bin/SHARModLoader.asi")
message(STATUS "======================")
