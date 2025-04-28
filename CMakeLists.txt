cmake_minimum_required(VERSION 3.5)

# Set the PROJECT_NAME, PROJECT_VERSION as well as other variable
project(boilerplate_asyncTaskScheduler
   VERSION 1.0.0
   DESCRIPTION "C++ asynchronous task scheduler boilerplate"
   LANGUAGES CXX C
)

### set readable summary for this version
set(PROJECT_VERSION_DESCRIPTION "Asynchronous task scheduler Test and probes")

### Options
option(BUILD_TEST "Build test-applications" ON)

# Defines the CMAKE_INSTALL_LIBDIR, CMAKE_INSTALL_BINDIR and many other useful macros.
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CMakeDependentOption)
include(CTest)

# Control where libraries and executables are placed during the build (development).
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")

#######################################################
### Compiler
#######################################################

# Disable C and C++ compiler extensions.
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

# Add packages
find_package(Threads REQUIRED)
find_package(spdlog REQUIRED)

# Build debug-configuration by default
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif ()

# Strip everything
add_link_options($<$<CONFIG:RELEASE>:-s>)

# Some options for the compiler
add_compile_options(
   $<$<CONFIG:DEBUG>:-g>
   $<$<CONFIG:RELEASE>:-O3>
   -Wall
   -Wextra
   -Werror=return-type
   -Werror=implicit-fallthrough
   -Wpedantic
   -Wconversion
   # Make switch-case statements with missing cases and no default an error.
   # By not adding the default-case you can enforce that all cases must be specified.
   -Werror=switch
   # Prevent "note: parameter passing for argument of type '...' changed in GCC 7.1"
   -Wno-psabi
)

#######################################################
### Add subdirectories
#######################################################
### library C++ support library
add_subdirectory(lib)

### application.
add_subdirectory(src)

# Check if we need to build the tests
if (BUILD_TEST)
  add_subdirectory(test)
endif ()

