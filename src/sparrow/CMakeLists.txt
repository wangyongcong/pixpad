set(SRC_COMMON 
	"common/any_stride_iterator.h"
	"common/image.cpp"
	"common/image.h"
	"common/log.cpp"
	"common/log.h"
	"common/unitest.h"
	"common/util.cpp"
	"common/util.h"
	"common/stb_image.h"
	"common/stb_image_resize.h"
	"common/stb_image_write.h"
) 

set(SRC_MATHEX 
	"mathex/floatmath.h"
	"mathex/vecmath.cpp"
	"mathex/vecmath.h"
	"mathex/xs_float.h"
) 
set(SRC_RENDERER 
	"renderer/clipping.cpp"
	"renderer/clipping.h"
	"renderer/index_buffer.h"
	"renderer/material.cpp"
	"renderer/material.h"
	"renderer/mesh.cpp"
	"renderer/mesh.h"
	"renderer/sampler.h"
	"renderer/sampler.cpp"
	"renderer/metric.cpp"
	"renderer/metric.h"
	"renderer/render_command.cpp"
	"renderer/render_command.h"
	"renderer/render_target.h"
	"renderer/renderer.cpp"
	"renderer/renderer.h"
	"renderer/spw_command.cpp"
	"renderer/spw_command.h"
	"renderer/spw_config.h"
	"renderer/spw_pipeline.cpp"
	"renderer/spw_pipeline.h"
	"renderer/spw_pipeline_wireframe.cpp"
	"renderer/spw_pipeline_wireframe.h"
	"renderer/spw_rasterizer.h"
	"renderer/spw_render_target.cpp"
	"renderer/spw_render_target.h"
	"renderer/spw_renderer.cpp"
	"renderer/spw_renderer.h"
	"renderer/surface.cpp"
	"renderer/surface.h"
	"renderer/tile.cpp"
	"renderer/tile.h"
	"renderer/vertex_buffer.cpp"
	"renderer/vertex_buffer.h"
	"renderer/vertex_layout.h"
) 
set(SRC_THREAD 
	# "thread/atomicops.h"
	"thread/disruptor.cpp"
	"thread/disruptor.h"
	# "thread/folly_queue.h"
	# "thread/lockfree_rb_q.h"
	# "thread/mpmc_queue.h"
	"thread/platform_info.cpp"
	"thread/platform_info.h"
	# "thread/readerwriterqueue.h"
	"thread/ring_queue.h"
	"thread/spin_lock.h"
	# "thread/spsc_queue.h"
) 
 
source_group("common" FILES ${SRC_COMMON}) 
source_group("mathex" FILES ${SRC_MATHEX}) 
source_group("renderer" FILES ${SRC_RENDERER}) 
source_group("thread" FILES ${SRC_THREAD}) 
 
include_directories(
	"common" 
	"mathex" 
	"renderer" 
	"thread" 
) 

add_library("sparrow" STATIC 
	${SRC_COMMON} 
	${SRC_MATHEX} 
	${SRC_RENDERER} 
	${SRC_THREAD} 
	${SRC_SCENE} 
)

