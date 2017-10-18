cmake_minimum_required(VERSION 3.3)
project(pixpad)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

#--------------------------------------------------
# setup 3rd libs
include(ExternalProject)

set(3RD_PATH "${CMAKE_SOURCE_DIR}/3rd")
set(3RD_INSTALL_PATH "${CMAKE_SOURCE_DIR}/install")

# boost
set(Boost_USE_STATIC_LIBS ON)
add_definitions(-DBOOST_ALL_NO_LIB)  # disable boost auto link
find_package(Boost 1.5 REQUIRED COMPONENTS program_options system filesystem)
include_directories(${Boost_INCLUDE_DIRS})

# zlib
# ExternalProject_Add(zlib
#     URL ${3RD_PATH}/zlib-1.2.8.tar.gz
#     CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${3RD_INSTALL_PATH}/zlib"
# )
# set(ZLIB_INCLUDE_DIR "${3RD_INSTALL_PATH}/zlib/include")
# link_directories(${3RD_INSTALL_PATH}/zlib/lib)

# libpng
# ExternalProject_Add(libpng
#     URL ${3RD_PATH}/libpng-1.6.21.tar.gz
#     CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${3RD_INSTALL_PATH}/libpng"
#     CMAKE_CACHE_DEFAULT_ARGS "-DZLIB_ROOT:string=${3RD_INSTALL_PATH}/zlib"
# )
# set(PNG_INCLUDE_DIR "${3RD_INSTALL_PATH}/libpng/include")
# link_directories(${3RD_INSTALL_PATH}/libpng/lib)
# set(PNG_LIBS libpng16_staticd zlibstaticd)
# add_dependencies(libpng zlib)

# openexr
set(BUILD_SHARED_LIBS OFF)
set(NAMESPACE_VERSIONING ON)
add_definitions("-DPLATFORM_WINDOWS")
set(ILMBASE_PATH ${3RD_PATH}/openexr/IlmBase)
include(IlmBaseConf)
include_directories(
	${ILMBASE_PATH}/Iex
	${ILMBASE_PATH}/IexMath
	${ILMBASE_PATH}/Imath
	${CMAKE_CURRENT_BINARY_DIR}/config
)
add_subdirectory(${ILMBASE_PATH}/Iex)
add_subdirectory(${ILMBASE_PATH}/IexMath)
add_subdirectory(${ILMBASE_PATH}/Imath)
# ExternalProject_Add(IlmBase
#     SOURCE_DIR ${3RD_PATH}/openexr/IlmBase
#     CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${3RD_INSTALL_PATH}/openexr"
# )
# set(OPENEXR_INCLUDE_DIR "${3RD_INSTALL_PATH}/openexr/include")
# link_directories(${3RD_INSTALL_PATH}/openexr/lib)
# set(OPENEXR_LIBS Imath-2_2 Half IexMath-2_2 Iex-2_2)

#--------------------------------------------------

set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin)
set(SRC_ROOT "${CMAKE_SOURCE_DIR}/src")

include_directories(
    ${3RD_INSTALL_PATH}
    ${SRC_ROOT}
)

# setup targets
add_subdirectory(src/sparrow)
add_dependencies(sparrow Iex IexMath Imath)
set(SPARROW_INCLUDE_DIR "${SRC_ROOT}/sparrow"
	"${SRC_ROOT}/sparrow/common" 
	"${SRC_ROOT}/sparrow/material" 
	"${SRC_ROOT}/sparrow/mathex" 
	"${SRC_ROOT}/sparrow/renderer" 
	"${SRC_ROOT}/sparrow/thread" 
	"${SRC_ROOT}/sparrow/scene"
)
include_directories(${SPARROW_INCLUDE_DIR})

# tests
add_subdirectory(src/testbed)

# pixpad
add_subdirectory(src/pixpad)

enable_testing()
add_test(test_line ${BIN_DIR}/testbed line)
add_test(test_box ${BIN_DIR}/testbed box)

