project(pixpad)
cmake_minimum_required(VERSION 2.8)

message(STATUS "Compiler: " ${CMAKE_CXX_COMPILER_ID})

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
	set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " /FS")
endif()

message(STATUS "Compiler flags: " ${CMAKE_CXX_FLAGS})

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	set(LIB_SUFFIX "d")
else()
	set(LIB_SUFFIX)
endif()
set(LIBRARY_OUTPUT_PATH  ${CMAKE_SOURCE_DIR}/lib)

INCLUDE_DIRECTORIES(
	${pixpad_SOURCE_DIR}
	${pixpad_SOURCE_DIR}/mathex
	${pixpad_SOURCE_DIR}/util
	${pixpad_SOURCE_DIR}/render
)

# setup lib mathex
set(mathex_src
	mathex/mathex.cpp
	mathex/transform.cpp
)
set(mathex_hdr
	mathex/mathex.h
	mathex/vecmath.h
	mathex/arithmetic_helper.h
	mathex/fixedp.h
	mathex/transform.h
)

# setup lib util
set(util_src
	util/util.cpp
)
set(util_hdr
	util/rect.h
	util/util.h
)

# setup sw renderer
set(render_src
	render/raster.cpp
)
set(render_hdr
	render/raster.h
)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	aux_source_directory(test TEST_LIST)
	add_executable(${PROJECT_NAME}_test
		${TEST_LIST}
		${mathex_src}
		${mathex_hdr}
		${util_src}
		${util_hdr}
		${render_src}
		${render_hdr}
	)
endif()

set(LIB_PIXPAD lib${PROJECT_NAME}${LIB_SUFFIX})
add_library(${LIB_PIXPAD}
	${TEST_LIST}
	${mathex_src}
	${mathex_hdr}
	${util_src}
	${util_hdr}
	${render_src}
	${render_hdr}
)
