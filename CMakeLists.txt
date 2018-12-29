cmake_minimum_required(VERSION 3.3)
project(pixpad)

set(CMAKE_CXX_STANDARD 14)
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

if(WIN32)
	# Warning C4819: The file contains a character that cannot be represented in the current code page
	add_definitions("/we4819")
endif()

# OpenEXR IlmBase
set(BUILD_SHARED_LIBS OFF)
set(BUILD_ILMBASE_STATIC ON)
set(NAMESPACE_VERSIONING ON)
set(ILMBASE_PATH ${3RD_PATH}/openexr/IlmBase)
set(ILMBASE_CONFIG_PATH ${ILMBASE_PATH}/config CACHE INTERNAL "")
set(ILMBASE_CONFIG_FILE ${ILMBASE_CONFIG_PATH}/IlmBaseConfig.h CACHE INTERNAL "")
if (NOT EXISTS ${ILMBASE_CONFIG_FILE})
	include(${ILMBASE_PATH}/cmake/OpenEXRVersion)
	include(IlmBaseConf)
endif()
include_directories(
	${ILMBASE_CONFIG_PATH}
	${ILMBASE_PATH}/Iex
	${ILMBASE_PATH}/IexMath
	${ILMBASE_PATH}/Imath
)
add_subdirectory(${ILMBASE_PATH}/Iex)
add_subdirectory(${ILMBASE_PATH}/IexMath)
add_subdirectory(${ILMBASE_PATH}/Imath)

#--------------------------------------------------

# definitions according build type
add_compile_definitions(
	# Debug
	$<$<CONFIG:Debug>:DEBUG>
	$<$<CONFIG:Debug>:LOG_SEVERITY_LEVEL=10>
	# MinSizeRel
	$<$<CONFIG:MinSizeRel>:RELEASE>
	$<$<CONFIG:MinSizeRel>:OPTIMIZE_SIZE>
	$<$<CONFIG:MinSizeRel>:LOG_SEVERITY_LEVEL=20>
	# RelWithDebInfo
	$<$<CONFIG:RelWithDebInfo>:RELEASE>
	$<$<CONFIG:RelWithDebInfo>:OPTIMIZE>
	$<$<CONFIG:RelWithDebInfo>:LOG_SEVERITY_LEVEL=20>
	# Release
	$<$<CONFIG:Release>:NDEBUG>
	$<$<CONFIG:Release>:RELEASE>
	$<$<CONFIG:Release>:OPTIMIZE>
	$<$<CONFIG:Release>:LOG_SEVERITY_LEVEL=40>
)

set(BIN_DIR ${CMAKE_SOURCE_DIR}/${CMAKE_BUILD_TYPE}/bin)
set(SRC_ROOT ${CMAKE_SOURCE_DIR}/src)
set(LIB_IMATH Iex_static IexMath_static Imath_static)

include_directories(
    ${3RD_INSTALL_PATH}
    ${SRC_ROOT}
)

if(WIN32)
	add_compile_options("/utf-8")
endif()

# setup targets
add_subdirectory(src/sparrow)
add_dependencies(sparrow ${LIB_IMATH})
set(SPARROW_INCLUDE_DIR 
	"${SRC_ROOT}/sparrow"
	"${SRC_ROOT}/sparrow/common" 
	"${SRC_ROOT}/sparrow/mathex" 
	"${SRC_ROOT}/sparrow/renderer" 
	"${SRC_ROOT}/sparrow/thread" 
)
include_directories(${SPARROW_INCLUDE_DIR})

# tests
add_subdirectory(src/testbed)

# pixpad
add_subdirectory(src/pixpad)
