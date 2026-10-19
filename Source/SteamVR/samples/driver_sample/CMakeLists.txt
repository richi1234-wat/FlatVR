cmake_minimum_required(VERSION 3.10)
project(driver_sample CXX)

set(CMAKE_CXX_STANDARD 17) # Change 14 to 17
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Define relative paths to the headers and libs
set(STEAMVR_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(STEAMVR_HEADERS "${STEAMVR_SOURCE_DIR}/headers")

if(NOT EXISTS "${STEAMVR_HEADERS}/openvr.h")
    message(FATAL_ERROR "Could not find openvr.h at ${STEAMVR_HEADERS}. Make sure the folder structure matches the original zip.")
endif()

# 2. Set up target
set(TARGET_NAME driver_sample)
set(SOURCES driverlog.cpp driver_sample.cpp)

# Add IniReader if it exists in this folder
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/IniReader/IniReader.cpp")
    list(APPEND SOURCES "IniReader/IniReader.cpp")
endif()

add_library(${TARGET_NAME} SHARED ${SOURCES})

# 3. Includes and Definitions
target_include_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${STEAMVR_HEADERS}
)
add_definitions(-DDRIVER_SAMPLE_EXPORTS)

# 4. Link against local OpenVR lib
target_link_libraries(${TARGET_NAME} PRIVATE
    ${CMAKE_DL_LIBS}
)

# 5. Output settings
set_target_properties(${TARGET_NAME} PROPERTIES
    PREFIX ""
    SUFFIX ".dll"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin/win64"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin/win64"
)