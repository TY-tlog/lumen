cmake_minimum_required(VERSION 3.28)

project(Lumen
    VERSION 0.0.1
    DESCRIPTION "Standalone interactive scientific plot viewer"
    LANGUAGES CXX
)

# ---------- C++ standard ----------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------- Build options ----------
option(LUMEN_BUILD_TESTS         "Build tests"                                  ON)
option(LUMEN_ENABLE_ASAN         "Enable AddressSanitizer (Debug only)"         OFF)
option(LUMEN_ENABLE_UBSAN        "Enable UndefinedBehaviorSanitizer (Debug only)" OFF)
option(LUMEN_ENABLE_WARNINGS     "Enable strict warnings"                       ON)
option(LUMEN_WARNINGS_AS_ERRORS  "Treat warnings as errors"                     ON)

# ---------- Output dirs ----------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ---------- Compile commands for clangd / clang-tidy ----------
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------- ccache ----------
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    message(STATUS "ccache found: ${CCACHE_PROGRAM}")
endif()

# ---------- Module path ----------
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# ---------- Compiler flags ----------
include(LumenCompilerOptions)

# ---------- Qt 6 ----------
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
)

qt_standard_project_setup()

# ---------- Sources ----------
add_subdirectory(src/lumen)

# ---------- Tests ----------
if(LUMEN_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ---------- Status summary ----------
message(STATUS "")
message(STATUS "==== Lumen build configuration ====")
message(STATUS "  Version:           ${PROJECT_VERSION}")
message(STATUS "  Build type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard:      ${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler:          ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Qt version:        ${Qt6_VERSION}")
message(STATUS "  Tests:             ${LUMEN_BUILD_TESTS}")
message(STATUS "  ASan:              ${LUMEN_ENABLE_ASAN}")
message(STATUS "  UBSan:             ${LUMEN_ENABLE_UBSAN}")
message(STATUS "  Warnings as errors: ${LUMEN_WARNINGS_AS_ERRORS}")
message(STATUS "===================================")
message(STATUS "")
