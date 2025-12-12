cmake_minimum_required(VERSION 3.20)
project(ProducerConsumer LANGUAGES C CXX)

# Always export compile_commands.json (for clangd, VSCode, etc.)
#set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # OK
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Export compile_commands.json for IDEs")

# Use C++17 everywhere
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default build type if not specified (Release / Debug)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Set default output directories (bin/ for executables, lib/ for libraries)
# Create bin and lib in project root directory (e.g., CMAKE_SOURCE_DIR)
#
# CMAKE_SOURCE_DIR is the full path to the root directory containing the
# top-level CMakeLists.txt file
#
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

# --- Universal output folder configuration ---
# Handle both single- and multi-config generators

# Single-config (e.g., Unix Makefiles, Ninja): Only one build
# configuration at a time (Debug, Release, etc.). Choose the 
# config when run CMake:
# $ cmake -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" ..
#
# Create bin and lib in ${CMAKE_SOURCE_DIR}/build and 
# ${CMAKE_SOURCE_DIR}/build/<preset>/ directory respectively,

# ${CMAKE_BINARY_DIR} is ${CMAKE_SOURCE_DIR}/build/<preset>
#
# CMAKE_BINARY_DIR is the full path to the root of the build tree, where
# CMake generates build files and the project's compiled outputs are stored
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

# Multi-config (Visual Studio, Xcode, Ninja Multi-Config)
# Generate multiple configurations in one project. Configuration should be
# picked at build time:
# $ cmake --build . --config Release
#
# This means Visual Studio has separate folders for each config like:
#  build/msvc/Debug/
#  build/msvc/Release/

if(CMAKE_CONFIGURATION_TYPES)
    foreach(OUTPUTCONFIG IN LISTS CMAKE_CONFIGURATION_TYPES)
        string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib/${OUTPUTCONFIG})
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib/${OUTPUTCONFIG})
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${CMAKE_BINARY_DIR}/../bin/${OUTPUTCONFIG})
    endforeach()
endif()

# --------------------------------------------------------------------
# Include and source directories
# --------------------------------------------------------------------
include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB SRC_FILES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

# --------------------------------------------------------------------
# Create executable
# --------------------------------------------------------------------
add_executable(${PROJECT_NAME} ${SRC_FILES})

# --------------------------------------------------------------------
# Optional: link with pthread and filesystem if needed
# --------------------------------------------------------------------
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

# For GCC < 9, may need to link filesystem explicitly
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
endif()

# --------------------------------------------------------------------
# Compiler detection + flags
# --------------------------------------------------------------------
message(STATUS "===========================================")
# if (CMAKE_CXX_COMPILER_ID MATCHES "Clang") OK
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "Using Clang compiler: ${CMAKE_CXX_COMPILER}")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra -Wpedantic
        -O2
        -Wshadow -Wconversion
        -Wnon-virtual-dtor -Wold-style-cast
    )
# elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU") OK
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Using GCC compiler: ${CMAKE_CXX_COMPILER}")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra -Wpedantic
        -O2
        -Wshadow -Wconversion
        -Wno-unused-parameter
        -fdiagnostics-color=always
    )
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(STATUS "Using Microsoft Visual C++ compiler")
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4 /permissive- /EHsc
    )
else()
    message(WARNING "Unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "===========================================")

#------------------------------------------------------------------------------
# Automatically copy or symlink build/<preset>/compile_commands.json to project root
# directory. IDE like clangd, VS Code IntelliSense, ale YouCompleteMe, Code
# analysis / refactoring tools expect this file in project root directory.
# For this to work, CMAKE_EXPORT_COMPILE_COMMANDS should be enabled in
# CMakePresets.json (Copy for per compiler-preset) and/or CMakeLists.txt
# (Always copy for all compilers).
#
# So when we run:
#   $ cmake --build --preset clang    or
#   $ cmake --build --preset gcc
#
# CMake automatically ensures that we have an up-to-date compile_commands.json
# in the project root.
#
# Creates a symbolic link for compile_commands.json at the project root
# (on Unix/Linux/macOS/Cygwin), instead of copying it each time.
# -----------------------------------------------------------------------------
if (CMAKE_EXPORT_COMPILE_COMMANDS)
    # Try to create symlink (preferred) or copy fallback
    if (UNIX OR CYGWIN) # Create symlink
        add_custom_target(link-compile-commands ALL
            COMMAND ${CMAKE_COMMAND} -E create_symlink
                ${CMAKE_BINARY_DIR}/compile_commands.json
                ${CMAKE_SOURCE_DIR}/compile_commands.json
            COMMENT "Linking compile_commands.json to project root"
        )
    else()  # Copy for windows (non-cygwin)
        add_custom_target(copy-compile-commands ALL
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_BINARY_DIR}/compile_commands.json
                ${CMAKE_SOURCE_DIR}/compile_commands.json
            COMMENT "Copying compile_commands.json to project root"
        )
    endif()
endif()
