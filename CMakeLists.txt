cmake_minimum_required(VERSION 3.3)
project(pixpad)

set(CMAKE_MODULE_PATH, "${CMAKE_SOURCE_DIR}/cmake")

enable_testing()

# setup 3rd libs
include(ExternalProject)

set(3RD_PATH "${CMAKE_SOURCE_DIR}/3rd")
set(3RD_INSTALL_PATH "${CMAKE_SOURCE_DIR}/install")

# zlib
set(ZLIB_PATH "${3RD_PATH}/zlib")
ExternalProject_Add(zlib
    URL ${3RD_PATH}/zlib-1.2.8.tar.gz
    PREFIX ${ZLIB_PATH}
    INSTALL_DIR ${3RD_INSTALL_PATH}/zlib
)
set(ZLIB_INCLUDE_DIR "${3RD_INSTALL_PATH}/zlib/include")
set(ZLIB_LIBRARY "${3RD_INSTALL_PATH}/zlib/lib")

# execute_process(
#     COMMAND cmake
# )

# OpenEXR
set(OPENEXR_PATH "${3RD_PATH}/openexr")

add_subdirectory(sparrow)
add_dependencies(sparrow zlib)

add_subdirectory(testbed)

