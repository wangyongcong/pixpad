project(engine)

set(PLATFORM_NAME "Windows" CACHE STRING "Platform Name")
set(RENDERER_NAME "Direct3D12" CACHE STRING "Renderer Name")

# default platform/renderer name
if(WIN32)
	set_property(CACHE PLATFORM_NAME PROPERTY STRINGS Windows)
	set_property(CACHE RENDERER_NAME PROPERTY STRINGS Direct3D12)
endif()

string(TOLOWER ${PLATFORM_NAME} PLATFORM_NAME_LOWER)
string(TOLOWER ${RENDERER_NAME} RENDERER_NAME_LOWER)

# -----------
# Common 模块
# -----------
set(COMMON_PUBLIC
	# Common module
	common/any_stride_iterator.h
	common/assertion_macros.h
	common/log_macros.h
	common/memory.h
	common/util_macros.h
	common/utility.h
)

set(COMMON_PRIVATE
	# Common module
	common/memory_tracking.cpp
	common/utility.cpp
)

set(COMMON_MODULE_SOURCE
	${COMMON_PUBLIC}
	${COMMON_PRIVATE}
)

# -----------
# MathEx 模块
# -----------
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

# -----------
# Game 模块
# -----------
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

# ---------------
# 平台模块
# ---------------
set(PLATFORM_PUBLIC
	platform/platform.h
)

set(SPECIFIC_PLATFORM_SOURCE
	platform/${PLATFORM_NAME}/${PLATFORM_NAME_LOWER}_application.h
	platform/${PLATFORM_NAME}/${PLATFORM_NAME_LOWER}_application.cpp
	platform/${PLATFORM_NAME}/${PLATFORM_NAME_LOWER}_window.h
	platform/${PLATFORM_NAME}/${PLATFORM_NAME_LOWER}_window.cpp
	platform/${PLATFORM_NAME}/${PLATFORM_NAME_LOWER}_platform.h
	platform/${PLATFORM_NAME}/${PLATFORM_NAME_LOWER}_platform.cpp
)

set(PLATFORM_MODULE_SOURCE
	${PLATFORM_PUBLIC}
	${SPECIFIC_PLATFORM_SOURCE}
)

if(PLATFORM_NAME STREQUAL "Windows")
	add_definitions(-DPLATFORM_WINDOWS)
endif() # Windows

# -----------
# 渲染模块
# -----------
set(RENDERER_PUBLIC
	renderer/renderer.h
)

set(RENDERER_PRIVATE
	renderer/image.h
	renderer/image.cpp
	renderer/ply.h
	renderer/ply.cpp
)

set(RENDERER_COMMON_SOURCE
	${RENDERER_PUBLIC}
	${RENDERER_PRIVATE}
)

if(RENDERER_NAME STREQUAL "Direct3D12")
	add_definitions(-DRENDERER_DIRECT3D12)
	set(SPECIFIC_RENDERER_SOURCE
		renderer/d3d12/d3d_helper.h
		renderer/d3d12/d3d_helper.cpp
		renderer/d3d12/renderer_d3d12.h
		renderer/d3d12/renderer_d3d12.cpp
	)
endif() # Direct3D 12

set(RENDERER_MODULE_SOURCE
	${RENDERER_PUBLIC}
	${RENDERER_PRIVATE}
	${SPECIFIC_RENDERER_SOURCE}
)

# ---------------
# 设置第三方库依赖
# ---------------
file(GLOB_RECURSE STB_SOURCE LIST_DIRECTORIES false
	${3RD_PATH}/stb/*.h
	${3RD_PATH}/stb/*.cpp
)

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

# ------------------
# 渲染器相关的第三方库
# ------------------

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
endif() # End Direct3D 12

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
source_group("3rd/D3D12Lib" FILES ${D3D12LIB_SOURCE})
source_group("3rd/D3D12MemoryAllocator" FILES ${D3D12_MEMORY_ALLOCATOR_SOURCE})
source_group("3rd/MemoryManager" FILES ${MEMORY_MANAGER_SOURCE})
source_group("3rd/rtm" FILES ${RTM_SOURCE})
source_group("3rd/stb" FILES ${STB_SOURCE})
source_group("Natvis" FILES ${NATVIS})

# ----------------------
# Windows resource files
# ----------------------

if(PLATFORM_NAME STREQUAL "Windows")
	set(RESOURCE_FILES 
		platform/windows/game.rc
		platform/windows/Resources/Icon/app_icon.ico
	)
	source_group("Resources" FILES platform/windows/game.rc)
	source_group("Resources/Icon" FILES platform/windows/Resources/Icon/app_icon.ico)
endif()

# --------
# 设置工程
# --------

option(USE_MEMORY_TRACKING "Enable memory tracking" "ON")

if (USE_MEMORY_TRACKING)
	add_definitions(-DUSE_MEMORY_TRACKING)
endif()

add_library(${PROJECT_NAME} SHARED
	${THIRD_PARTY_SOURCE}
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
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
	PUBLIC ${THIRD_PARTY_PUBLIC}
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC d3d12.lib
	PUBLIC dxgi.lib
	PUBLIC dxguid.lib
	# 如果是 static lib 要将资源文件链接到库里面
	# PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/GameFramework.dir/${CMAKE_CFG_INTDIR}/GameFramework.res
)