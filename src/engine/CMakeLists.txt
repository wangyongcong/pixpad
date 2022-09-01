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
# 设置模块源码
# -----------

set(GAME_FRAMEWORK_PUBLIC 
	game/public
)

set(GAME_FRAMEWORK_PRIVATE 
	game/private
)

set(PUBLIC_SOURCE
	# Game module
	game/public/game_framework.h
	game/public/game_application.h
	game/public/game_instance.h
	game/public/renderer.h
	game/public/game_window.h
	game/public/assertion_macros.h
	game/public/log_macros.h
	game/public/util_macros.h
	game/public/utility.h
	game/public/memory.h
	game/public/platform.h
)

set(PRIVATE_SOURCE
	# Game module
	game/private/game_framework_pch.h
	game/private/game_application.cpp
	game/private/utility.cpp
	game/private/memory_tracking.cpp
)

set(MODULE_SOURCE
	${PUBLIC_SOURCE}
	${PRIVATE_SOURCE}
)

# ---------------
# 选择不同的操作系统
# ---------------
set(PLATFORM_SOURCE
	platform/${PLATFORM_NAME}/${PLATFORM_NAME_LOWER}_application.h
	platform/${PLATFORM_NAME}/${PLATFORM_NAME_LOWER}_application.cpp
	platform/${PLATFORM_NAME}/${PLATFORM_NAME_LOWER}_window.h
	platform/${PLATFORM_NAME}/${PLATFORM_NAME_LOWER}_window.cpp
	platform/${PLATFORM_NAME}/${PLATFORM_NAME_LOWER}_platform.h
	platform/${PLATFORM_NAME}/${PLATFORM_NAME_LOWER}_platform.cpp
)
set(PLATFORM_PRIVATE
	platform/${PLATFORM_NAME}
)

if(PLATFORM_NAME STREQUAL "Windows")
	add_definitions(-DPLATFORM_WINDOWS)
endif() # Windows

# ---------------
# 选择不同的渲染器
# ---------------

if(RENDERER_NAME STREQUAL "Direct3D12")
add_definitions(-DRENDERER_DIRECT3D12)
set(RENDERER_SOURCE
	renderer/${RENDERER_NAME}/d3d_helper.h
	renderer/${RENDERER_NAME}/d3d_helper.cpp
	renderer/${RENDERER_NAME}/renderer_d3d12.h
	renderer/${RENDERER_NAME}/renderer_d3d12.cpp
)
set(RENDERER_PRIVATE
	renderer/${RENDERER_NAME}
)
endif() # Direct3D 12

file(GLOB_RECURSE STB_SOURCE LIST_DIRECTORIES false
	${3RD_PATH}/stb/*.h
	${3RD_PATH}/stb/*.cpp
)

# ---------------
# 设置第三方库依赖
# ---------------

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


# game framework group
source_group("game/public" FILES ${PUBLIC_SOURCE})
source_group("game/private" FILES ${PRIVATE_SOURCE})
# render group
source_group("renderer/${RENDERER_NAME}" FILES ${RENDERER_SOURCE})
# platform group
source_group("platform/${PLATFORM_NAME}" FILES ${PLATFORM_SOURCE})
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
	${MODULE_SOURCE}
	${PLATFORM_SOURCE}
	${RENDERER_SOURCE}
	${RESOURCE_FILES}
	${STB_SOURCE}
)

target_precompile_headers(${PROJECT_NAME}
	PRIVATE game/private/game_framework_pch.h
)

target_include_directories(${PROJECT_NAME}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
	PUBLIC ${THIRD_PARTY_PUBLIC}
	PRIVATE ${GAME_FRAMEWORK_PRIVATE}
	PUBLIC ${GAME_FRAMEWORK_PUBLIC}
	PRIVATE ${PLATFORM_PRIVATE}
	PRIVATE ${RENDERER_PRIVATE}
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC d3d12.lib
	PUBLIC dxgi.lib
	PUBLIC dxguid.lib
	# 如果是 static lib 要将资源文件链接到库里面
	# PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/GameFramework.dir/${CMAKE_CFG_INTDIR}/GameFramework.res
)