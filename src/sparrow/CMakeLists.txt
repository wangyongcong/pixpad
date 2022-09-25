project(sparrow)

set(ENGINE_PATH ${SRC_ROOT}/engine)
set(SPARROW_PATH ${SRC_ROOT}/engine/renderer/sparrow)

set(SRC_COMMON 
	${ENGINE_PATH}/common/any_stride_iterator.h
	${ENGINE_PATH}/common/utility.h
	${ENGINE_PATH}/common/utility.cpp
	${ENGINE_PATH}/common/memory_tracking.cpp
	${ENGINE_PATH}/renderer/image.h
	${ENGINE_PATH}/renderer/image.cpp
	${ENGINE_PATH}/renderer/ply.h
	${ENGINE_PATH}/renderer/ply.cpp
	${ENGINE_PATH}/renderer/vertex_layout.h
	${ENGINE_PATH}/renderer/index_buffer.h
	${ENGINE_PATH}/renderer/vertex_buffer.h
	${ENGINE_PATH}/renderer/vertex_buffer.cpp
	${ENGINE_PATH}/renderer/mesh.h
	${ENGINE_PATH}/renderer/mesh.cpp
) 

set(SRC_STB
	${3RD_PATH}/stb/stb_image.h
	${3RD_PATH}/stb/stb_image_resize.h
	${3RD_PATH}/stb/stb_image_write.h
	${3RD_PATH}/stb/stb_log.h
	${3RD_PATH}/stb/module.cpp
)

set(SRC_MATHEX 
	${ENGINE_PATH}/mathex/floatmath.h
	${ENGINE_PATH}/mathex/vecmath.cpp
	${ENGINE_PATH}/mathex/vecmath.h
	${ENGINE_PATH}/mathex/xs_float.h
)

set(SRC_CORE
	${SPARROW_PATH}/core/spw_config.h
	${SPARROW_PATH}/core/spw_tile_buffer.h
	${SPARROW_PATH}/core/spw_rasterizer.h
	${SPARROW_PATH}/core/spw_rasterizer.cpp
	${SPARROW_PATH}/core/spw_tile.h
	${SPARROW_PATH}/core/spw_tile.cpp
	${SPARROW_PATH}/core/spw_pipeline2.h
	${SPARROW_PATH}/core/spw_pipeline2.cpp
)

set(SRC_RENDERER 
	${SPARROW_PATH}/renderer/clipping.cpp
	${SPARROW_PATH}/renderer/clipping.h
	${SPARROW_PATH}/renderer/material.cpp
	${SPARROW_PATH}/renderer/material.h
	${SPARROW_PATH}/renderer/sampler.h
	${SPARROW_PATH}/renderer/sampler.cpp
	${SPARROW_PATH}/renderer/metric.cpp
	${SPARROW_PATH}/renderer/metric.h
	${SPARROW_PATH}/renderer/render_command.cpp
	${SPARROW_PATH}/renderer/render_command.h
	${SPARROW_PATH}/renderer/render_target.h
	${SPARROW_PATH}/renderer/renderer.cpp
	${SPARROW_PATH}/renderer/renderer.h
	${SPARROW_PATH}/renderer/spw_command.cpp
	${SPARROW_PATH}/renderer/spw_command.h
	${SPARROW_PATH}/renderer/spw_pipeline.cpp
	${SPARROW_PATH}/renderer/spw_pipeline.h
	${SPARROW_PATH}/renderer/spw_render_target.cpp
	${SPARROW_PATH}/renderer/spw_render_target.h
	${SPARROW_PATH}/renderer/spw_renderer.cpp
	${SPARROW_PATH}/renderer/spw_renderer.h
	${SPARROW_PATH}/renderer/surface.cpp
	${SPARROW_PATH}/renderer/surface.h
	${SPARROW_PATH}/renderer/tile.cpp
	${SPARROW_PATH}/renderer/tile.h
	${SPARROW_PATH}/renderer/shader_api.h
	${SPARROW_PATH}/renderer/shader_api.cpp
) 
set(SRC_THREAD 
	# thread/atomicops.h
	${SPARROW_PATH}/thread/disruptor.cpp
	${SPARROW_PATH}/thread/disruptor.h
	# thread/folly_queue.h
	# thread/lockfree_rb_q.h
	# thread/mpmc_queue.h
	${SPARROW_PATH}/thread/platform_info.cpp
	${SPARROW_PATH}/thread/platform_info.h
	# thread/readerwriterqueue.h
	${SPARROW_PATH}/thread/ring_queue.h
	${SPARROW_PATH}/thread/spin_lock.h
	# thread/spsc_queue.h
) 
 
source_group(common FILES ${SRC_COMMON}) 
source_group(mathex FILES ${SRC_MATHEX})
source_group(core FILES ${SRC_CORE})
source_group(renderer FILES ${SRC_RENDERER})
source_group(thread FILES ${SRC_THREAD}) 
source_group(stb FILES ${SRC_STB}) 


include_directories(
	${ENGINE_PATH}
	${ENGINE_PATH}/common
	${ENGINE_PATH}/renderer
	${ENGINE_PATH}/mathex
	${SPARROW_PATH}/core
	${SPARROW_PATH}/renderer 
	${SPARROW_PATH}/thread 
) 

add_library(sparrow STATIC 
	${SRC_COMMON} 
	${SRC_MATHEX}
	${SRC_CORE}
	${SRC_RENDERER} 
	${SRC_THREAD} 
	${SRC_SCENE} 
	${SRC_STB}
)

