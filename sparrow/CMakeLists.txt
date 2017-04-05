set(SRC_COMMON
	"$LIST_FILES(common)"
)
set(SRC_MATERIAL
	"$LIST_FILES(material)"
)
set(SRC_MATHEX
	"$LIST_FILES(mathex)"
)
set(SRC_RENDERER
	"$LIST_FILES(renderer)"
)
set(SRC_THREAD
	"therad/platform_info.h"
	"thread/platform_info.cpp"
	"thread/disruptor.h"
	"thread/disruptor.cpp"
)
set(SRC_SCENE
	"$LIST_FILES(scene)"
)

source_group("common" FILES ${SRC_COMMON})
source_group("material" FILES ${SRC_MATERIAL})
source_group("mathex" FILES ${SRC_MATHEX})
source_group("renderer" FILES ${SRC_RENDERER})
source_group("thread" FILES ${SRC_THREAD})
source_group("scene" FILES ${SRC_SCENE})

include_directories("."
	"common"
	"material"
	"mathex"
	"renderer"
	"thread"
	"scene"
)

add_library("sparrow" STATIC
	${SRC_COMMON}
	${SRC_MATERIAL}
	${SRC_MATHEX}
	${SRC_RENDERER}
	${SRC_THREAD}
	${SCENE}
)