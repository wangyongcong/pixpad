set(SRC_TESTBED
	"test_main.cpp"
	"test_box.h"
	"test_collada.h"
	"test_line.h"
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
)
