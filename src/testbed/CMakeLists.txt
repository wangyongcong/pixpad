set(SRC_TESTBED
	"test_main.cpp"
	"test.h"
	"test.cpp"
	"test_line.h"
	"test_collada.h"
	"test_box.h"
	"test_texture.h"
	"test_mipmap.h"
)

add_executable("testbed" ${SRC_TESTBED})

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
)
