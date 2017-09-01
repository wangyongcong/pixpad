set(SRC_COMMON
	"test_main.cpp"
	"test.h"
	"test.cpp"
)

set(SRC_TESTBED
	"test_line.h"
	"test_collada.h"
	"test_box.h"
	"test_texture.h"
	"test_mipmap.h"
)

set(SRC_MATERIAL
	"material/mtl_color.h"
	"material/mtl_diffuse.h"
)

source_group("common" FILES ${SRC_COMMON})
source_group("material" FILES ${SRC_MATERIAL})
source_group("test" FILES ${SRC_TESTBED})

include_directories(
	"material"
)

add_executable("testbed" ${SRC_COMMON} ${SRC_MATERIAL} ${SRC_TESTBED})

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
