cmake_minimum_required(VERSION 2.8)
project(pixpad)

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FS")
endif()

# set ouput path
set(LIBRARY_OUTPUT_PATH  ${CMAKE_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

INCLUDE_DIRECTORIES(
	${pixpad_SOURCE_DIR}
	${pixpad_SOURCE_DIR}/mathex
	${pixpad_SOURCE_DIR}/render
)
include_directories(3rd/include)

# setup OpenEXR
find_library(LIB_IMATH Imath-2_2 PATHS ${CMAKE_SOURCE_DIR}/3rd/lib)
# list OpenEXR headers in project
include(OpenEXR.txt)

# setup sw renderer
set(render_src
	render/vecmath.cpp
	render/raster.cpp
	render/surface.cpp
	render/gen_mesh.cpp
)
set(render_hdr
	render/vecmath.h
	render/vector.h
	render/matrix.h
	render/raster.h
	render/surface.h
	render/gen_mesh.h
)

set(LIB_PIXPAD ${PROJECT_NAME}${LIB_SUFFIX})
add_library(${LIB_PIXPAD}
	${render_src}
	${render_hdr}
	${OpenEXR_src}
	${OpenEXR_hdr}
)

set(test_src
	test/main.cpp
	test/test_sampler.h
	test/test_sampler.cpp
	render/test_matrix.cpp
	render/test_vector.cpp
)
set(TEST ${PROJECT_NAME}_test)
add_executable(${TEST}
	${test_src}
)
target_link_libraries(${TEST} ${LIB_PIXPAD})
target_link_libraries(${TEST} ${LIB_IMATH})

add_subdirectory(view)