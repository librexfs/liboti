cmake_minimum_required(VERSION 3.5)

project(oti C)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include/platform-dependent)

file(GLOB SRC_PLATFORM ${CMAKE_CURRENT_SOURCE_DIR}/linux_platform/*.c)
file(GLOB SRC_OTI_LIB
    ${CMAKE_CURRENT_SOURCE_DIR}/../../source/APIs/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../source/core/*.c
)
# Remove unused source files
list(REMOVE_ITEM SRC_OTI_LIB ${CMAKE_CURRENT_SOURCE_DIR}/../../source/APIs/RemovePICC.c)
list(REMOVE_ITEM SRC_OTI_LIB ${CMAKE_CURRENT_SOURCE_DIR}/../../source/core/TransparentTransportLayerControl.c)
list(APPEND SRC_OTI_LIB ${SRC_PLATFORM})

option(LIB_LOG_ENABLED "Oti lib logs" ON)

if(LIB_LOG_ENABLED)
    add_definitions(-DOTI_LIB_LOG_ENABLED)
endif()

if(BUILD_SHARED)
    add_library(oti SHARED ${SRC_OTI_LIB})
else()
    add_library(oti STATIC ${SRC_OTI_LIB})
endif()
