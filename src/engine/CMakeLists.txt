project(engine)

option(USE_MEMORY_TRACKING "Enable memory tracking" ON)

# default platform/renderer name
if(WIN32)
	set(PLATFORM_NAME "Windows" CACHE STRING "Platform Name")
	set(RENDERER_NAME "Direct3D12" CACHE STRING "Renderer Name")
elseif(APPLE)
	set(PLATFORM_NAME "Mac" CACHE STRING "Platform Name")
	set(RENDERER_NAME "Metal" CACHE STRING "Renderer Name")
endif()

set_property(CACHE PLATFORM_NAME PROPERTY STRINGS Windows Mac)
set_property(CACHE RENDERER_NAME PROPERTY STRINGS Direct3D12 Metal)

string(TOLOWER ${PLATFORM_NAME} PLATFORM_NAME_LOWER)
string(TOLOWER ${RENDERER_NAME} RENDERER_NAME_LOWER)

if(BUILD_SHARED_LIBS)
	set(ENGINE_PUBLIC_DEFINITIONS ${ENGINE_PUBLIC_DEFINITIONS} -DENGINE_SHARED_LIBS -DSTB_SHARED_LIBS)
else()
	set(ENGINE_PUBLIC_DEFINITIONS ${ENGINE_PUBLIC_DEFINITIONS} -DENGINE_STATIC_LIBS)
endif()

# -------------------------------------------------------------------
# Engine 代码
# -------------------------------------------------------------------
set(ENGINE_SOURCE
	engine.h
)

# -------------------------------------------------------------------
# Common 模块
# -------------------------------------------------------------------
set(COMMON_PUBLIC
	# Common module
	common/any_stride_iterator.h
	common/assertion_macros.h
	common/log_macros.h
	common/memory.h
	common/common_macros.h
	common/utility.h
	common/task_thread.h
)

set(COMMON_PRIVATE
	# Common module
	common/memory_tracking.cpp
	common/utility.cpp
	common/task_thread.cpp
)

set(COMMON_MODULE_SOURCE
	${COMMON_PUBLIC}
	${COMMON_PRIVATE}
)

# -------------------------------------------------------------------
# MathEx 模块
# -------------------------------------------------------------------
set(MATHEX_PUBLIC
	mathex/floatmath.h
	mathex/vecmath.h
	mathex/xs_float.h
)

set(MATHEX_PRIVATE
	mathex/vecmath.cpp
)

set(MATHEX_MODULE_SOURCE
	${MATHEX_PUBLIC}
	${MATHEX_PRIVATE}
)

# -------------------------------------------------------------------
# Game 模块
# -------------------------------------------------------------------
set(GAME_PUBLIC
	# Game module
	game/game_application.h
	game/game_instance.h
	game/game_window.h
)

set(GAME_PRIVATE
	# Game module
	game/game_application.cpp
)

set(GAME_MODULE_SOURCE
	${GAME_PUBLIC}
	${GAME_PRIVATE}
)

# -------------------------------------------------------------------
# 平台模块
# -------------------------------------------------------------------
set(PLATFORM_PUBLIC
	platform/platform.h
)

if(PLATFORM_NAME STREQUAL "Mac")
# @todo support MacOS 
else()
set(SPECIFIC_PLATFORM_SOURCE
	platform/${PLATFORM_NAME}/${PLATFORM_NAME_LOWER}_application.h
	platform/${PLATFORM_NAME}/${PLATFORM_NAME_LOWER}_application.cpp
	platform/${PLATFORM_NAME}/${PLATFORM_NAME_LOWER}_window.h
	platform/${PLATFORM_NAME}/${PLATFORM_NAME_LOWER}_window.cpp
	platform/${PLATFORM_NAME}/${PLATFORM_NAME_LOWER}_platform.h
	platform/${PLATFORM_NAME}/${PLATFORM_NAME_LOWER}_platform.cpp
)
endif()

set(PLATFORM_MODULE_SOURCE
	${PLATFORM_PUBLIC}
	${SPECIFIC_PLATFORM_SOURCE}
)

if(PLATFORM_NAME STREQUAL "Windows")
	set(ENGINE_PUBLIC_DEFINITIONS ${ENGINE_PUBLIC_DEFINITIONS} -DPLATFORM_WINDOWS)
elseif(PLATFORM_NAME STREQUAL "Mac")
	set(ENGINE_PUBLIC_DEFINITIONS ${ENGINE_PUBLIC_DEFINITIONS} -DPLATFORM_MAC)
endif()

# -------------------------------------------------------------------
# 渲染模块
# -------------------------------------------------------------------
set(RENDERER_PUBLIC
	renderer/renderer.h
)

set(RENDERER_PRIVATE
	renderer/image.h
	renderer/image.cpp
	renderer/ply.h
	renderer/ply.cpp
	renderer/vertex_layout.h
	renderer/vertex_buffer.h
	renderer/vertex_buffer.cpp
	renderer/index_buffer.h
	renderer/mesh.h
	renderer/mesh.cpp
)

set(RENDERER_COMMON_SOURCE
	${RENDERER_PUBLIC}
	${RENDERER_PRIVATE}
)

if(RENDERER_NAME STREQUAL "Direct3D12")
	set(ENGINE_PUBLIC_DEFINITIONS ${ENGINE_PUBLIC_DEFINITIONS} -DRENDERER_DIRECT3D12)
	add_definitions(-DRENDERER_DIRECT3D12)
	set(SPECIFIC_RENDERER_SOURCE
		renderer/d3d12/d3d_helper.h
		renderer/d3d12/d3d_helper.cpp
		renderer/d3d12/d3d12_type.h
		renderer/d3d12/renderer_d3d12.h
		renderer/d3d12/renderer_d3d12.cpp
		renderer/d3d12/resource_loader.h
		renderer/d3d12/resource_loader.cpp
	)
endif() # Direct3D 12

set(RENDERER_MODULE_SOURCE
	${RENDERER_PUBLIC}
	${RENDERER_PRIVATE}
	${SPECIFIC_RENDERER_SOURCE}
)

# -------------------------------------------------------------------
# Sparrow 渲染器
# -------------------------------------------------------------------
set(SPARROW_PATH renderer/sparrow)

set(SPARROW_RENDERER_PUBLIC 
	${SPARROW_PATH}/clipping.h
	${SPARROW_PATH}/material.h
	${SPARROW_PATH}/sampler.h
	${SPARROW_PATH}/metric.h
	${SPARROW_PATH}/render_command.h
	${SPARROW_PATH}/render_target.h
	${SPARROW_PATH}/renderer.h
	${SPARROW_PATH}/spw_config.h
	${SPARROW_PATH}/spw_command.h
	${SPARROW_PATH}/spw_pipeline.h
	${SPARROW_PATH}/spw_pipeline2.h
	${SPARROW_PATH}/spw_render_target.h
	${SPARROW_PATH}/spw_renderer.h
	${SPARROW_PATH}/spw_rasterizer.h
	${SPARROW_PATH}/spw_tile.h
	${SPARROW_PATH}/spw_tile_buffer.h
	${SPARROW_PATH}/surface.h
	${SPARROW_PATH}/tile.h
	${SPARROW_PATH}/shader_api.h
) 

set(SPARROW_RENDERER_PRIVATE
	${SPARROW_PATH}/private/clipping.cpp
	${SPARROW_PATH}/private/material.cpp
	${SPARROW_PATH}/private/sampler.cpp
	${SPARROW_PATH}/private/metric.cpp
	${SPARROW_PATH}/private/render_command.cpp
	${SPARROW_PATH}/private/renderer.cpp
	${SPARROW_PATH}/private/spw_command.cpp
	${SPARROW_PATH}/private/spw_pipeline.cpp
	${SPARROW_PATH}/private/spw_pipeline2.cpp
	${SPARROW_PATH}/private/spw_render_target.cpp
	${SPARROW_PATH}/private/spw_renderer.cpp
	${SPARROW_PATH}/private/spw_rasterizer.cpp
	${SPARROW_PATH}/private/spw_tile.cpp
	${SPARROW_PATH}/private/surface.cpp
	${SPARROW_PATH}/private/tile.cpp
	${SPARROW_PATH}/private/shader_api.cpp
)

set(SPARROW_THREAD 
	# ${SPARROW_PATH}/thread/atomicops.h
	${SPARROW_PATH}/thread/disruptor.cpp
	${SPARROW_PATH}/thread/disruptor.h
	# ${SPARROW_PATH}/thread/folly_queue.h
	# ${SPARROW_PATH}/thread/lockfree_rb_q.h
	# ${SPARROW_PATH}/thread/mpmc_queue.h
	${SPARROW_PATH}/thread/platform_info.cpp
	${SPARROW_PATH}/thread/platform_info.h
	# ${SPARROW_PATH}/thread/readerwriterqueue.h
	${SPARROW_PATH}/thread/ring_queue.h
	${SPARROW_PATH}/thread/spin_lock.h
	# ${SPARROW_PATH}/thread/spsc_queue.h
) 

source_group(renderer/sparrow/public FILES ${SPARROW_RENDERER_PUBLIC})
source_group(renderer/sparrow/private FILES ${SPARROW_RENDERER_PRIVATE})
source_group(renderer/sparrow/thread FILES ${SPARROW_THREAD}) 

set(SPARROW_MODULE ${SPARROW_RENDERER_PUBLIC} ${SPARROW_RENDERER_PRIVATE} ${SPARROW_THREAD})
set(RENDERER_MODULE_SOURCE ${RENDERER_MODULE_SOURCE} ${SPARROW_MODULE})

# -------------------------------------------------------------------
# 设置第三方库依赖
# -------------------------------------------------------------------
file(GLOB_RECURSE STB_SOURCE LIST_DIRECTORIES false
	${3RD_PATH}/stb/*.h
	${3RD_PATH}/stb/*.cpp
)
# export stb api
if(BUILD_SHARED_LIBS)
	add_definitions(-Dstb_EXPORTS)
endif()


set(MEMORY_MANAGER_PUBLIC ${3RD_PATH}/MemoryManager)
set(MEMORY_MANAGER_SOURCE
	${3RD_PATH}/MemoryManager/mmgr.cpp
	${3RD_PATH}/MemoryManager/mmgr.h
	${3RD_PATH}/MemoryManager/nommgr.h
)

# real time math lib
file(GLOB_RECURSE RTM_INCLUDE_FILES LIST_DIRECTORIES false
	${3RD_PATH}/rtm/includes/*.h
)

set(RTM_SOURCE
	${RTM_INCLUDE_FILES}
	${3RD_PATH}/rtm/tools/vs_visualizers/rtm.natvis
)

set(THIRD_PARTY_PUBLIC
	${3RD_PATH}
)

set(THIRD_PARTY_SOURCE
	${MEMORY_MANAGER_SOURCE}
	${RTM_SOURCE}
)

# -------------------------------------------------------------------
# 渲染器相关的第三方库
# -------------------------------------------------------------------

if(RENDERER_NAME STREQUAL "Direct3D12")
# D3D12 helper library
set(D3D12LIB_PUBLIC ${3RD_PATH}/D3DLib)
set(D3D12LIB_SOURCE
	${3RD_PATH}/D3DLib/d3dx12.h
)

# D3D12 memory allocation lib
set(D3D12_MEMORY_ALLOCATOR_PUBLIC ${3RD_PATH}/D3D12MemoryAllocator)
set(D3D12_MEMORY_ALLOCATOR_SOURCE
	${3RD_PATH}/D3D12MemoryAllocator/D3D12MemAlloc.h
	${3RD_PATH}/D3D12MemoryAllocator/D3D12MemAlloc.cpp
	${3RD_PATH}/D3D12MemoryAllocator/D3D12MemAlloc.natvis
)

set(THIRD_PARTY_PUBLIC
	${THIRD_PARTY_PUBLIC}
	${D3D12LIB_PUBLIC}
	${D3D12_MEMORY_ALLOCATOR_PUBLIC}
)

set(THIRD_PARTY_SOURCE
	${THIRD_PARTY_SOURCE}
	${D3D12LIB_SOURCE}
	${D3D12_MEMORY_ALLOCATOR_SOURCE}
)

source_group("3rd/D3D12Lib" FILES ${D3D12LIB_SOURCE})
source_group("3rd/D3D12MemoryAllocator" FILES ${D3D12_MEMORY_ALLOCATOR_SOURCE})
endif() # End Direct3D 12

# engine group
source_group("engine" FILES ${ENGINE_SOURCE})
# common group
source_group("common" FILES ${COMMON_MODULE_SOURCE})
# math group
source_group("mathex" FILES ${MATHEX_MODULE_SOURCE})
# game framework group
source_group("game" FILES ${GAME_MODULE_SOURCE})
# renderer group
source_group("renderer" FILES ${RENDERER_PUBLIC} FILES ${RENDERER_PRIVATE})
source_group("renderer/${RENDERER_NAME}" FILES ${SPECIFIC_RENDERER_SOURCE})
# platform group
source_group("platform" FILES ${PLATFORM_PUBLIC} FILES ${PLATFORM_PRIVATE})
source_group("platform/${PLATFORM_NAME}" FILES ${SPECIFIC_PLATFORM_SOURCE})
# third parth group
source_group("3rd/MemoryManager" FILES ${MEMORY_MANAGER_SOURCE})
source_group("3rd/rtm" FILES ${RTM_SOURCE})
source_group("3rd/stb" FILES ${STB_SOURCE})
source_group("Natvis" FILES ${NATVIS})

# -------------------------------------------------------------------
# Windows resource files
# -------------------------------------------------------------------

if(PLATFORM_NAME STREQUAL "Windows")
	set(RESOURCE_FILES 
		platform/windows/game.rc
		platform/windows/Resources/Icon/app_icon.ico
	)
	source_group("Resources" FILES platform/windows/game.rc)
	source_group("Resources/Icon" FILES platform/windows/Resources/Icon/app_icon.ico)
endif()

# -------------------------------------------------------------------
# 设置工程
# -------------------------------------------------------------------

if(USE_MEMORY_TRACKING)
	add_definitions(-DUSE_MEMORY_TRACKING)
endif()

add_library(${PROJECT_NAME} STATIC
	${THIRD_PARTY_SOURCE}
	${ENGINE_SOURCE}
	${COMMON_MODULE_SOURCE}
	${MATHEX_MODULE_SOURCE}
	${GAME_MODULE_SOURCE}
	${PLATFORM_MODULE_SOURCE}
	${RENDERER_MODULE_SOURCE}
	${RESOURCE_FILES}
	${STB_SOURCE}
	${NATVIS}
)

target_precompile_headers(${PROJECT_NAME}
	PRIVATE engine_pch.h
)

target_include_directories(${PROJECT_NAME}
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
	PUBLIC ${THIRD_PARTY_PUBLIC}
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC d3d12.lib
	PUBLIC dxgi.lib
	PUBLIC dxguid.lib
	absl::flat_hash_map
	absl::flat_hash_set
	absl::strings
	absl::status
)

if(NOT BUILD_SHARED_LIBS)
	# 如果是 static lib 要将资源文件链接到库里面
	target_link_libraries(${PROJECT_NAME}
		PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/engine.dir/${CMAKE_CFG_INTDIR}/game.res
	)
endif()

target_compile_definitions(${PROJECT_NAME}
	PUBLIC ${ENGINE_PUBLIC_DEFINITIONS}
)
