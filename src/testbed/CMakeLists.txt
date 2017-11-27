# build as stand alone executable
option(TEST_STAND_ALONE "Test program build as executable" OFF)

set(SRC_COMMON
	"test_main.cpp"
	"test.h"
	"test.cpp"
	"test_command.cpp"
)

set(SRC_TESTBED
	"test_line.cpp"
	"test_box.cpp"
	"test_texture.cpp"
	"test_mipmap.cpp"
	"test_depth.cpp"
	"test_wireframe.cpp"
)

set(SRC_MATERIAL
	"material/mtl_color.h"
	"material/mtl_diffuse.h"
	"material/mtl_wireframe.h"
)

source_group("common" FILES ${SRC_COMMON})
source_group("material" FILES ${SRC_MATERIAL})
source_group("test" FILES ${SRC_TESTBED})

include_directories(
	"material"
)

if(TEST_STAND_ALONE)
	add_executable("testbed" ${SRC_COMMON} ${SRC_MATERIAL} ${SRC_TESTBED})
	# message(STATUS "testbed stand alone executable")
	set_target_properties(testbed
		PROPERTIES
		VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
	)	
else()
	add_library(testbed SHARED ${SRC_COMMON} ${SRC_MATERIAL} ${SRC_TESTBED})
	# message(STATUS "testbed shared lib")
endif()

set_target_properties(testbed
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)

target_link_libraries(testbed 
	sparrow 
	${PNG_LIBS}
	${OPENEXR_LIBS}
	${Boost_PROGRAM_OPTIONS_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${Boost_FILESYSTEM_LIBRARY}
)
