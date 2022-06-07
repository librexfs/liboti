# Root project for OTI Saturn library
cmake_minimum_required(VERSION 3.5)

project(oti_reader C)

# Default to Release mode
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

option(BUILD_WITH_EXAMPLES "Build examples" OFF)
option(BUILD_SHARED "Build SHARED library" ON)

# Add platform specific impl
if(UNIX)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/platform/linux-gcc)
else(
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/win32-visual/windows_platform)
)
endif()

message("BUILD_WITH_EXAMPLES=" ${BUILD_WITH_EXAMPLES} )
message("BUILD_SHARED=" ${BUILD_SHARED} )

if(BUILD_WITH_EXAMPLES)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)
endif()

install(TARGETS oti LIBRARY DESTINATION lib)