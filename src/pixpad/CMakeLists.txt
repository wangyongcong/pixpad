project(pixpad)

set(SRC_PIXPAD
	# app_config.h
	# console.cpp
	# console_log.h
	# image_frame.cpp
)

set(SRC_IMGUI
	imgui/imconfig.h
	imgui/imgui.h
	imgui/imgui.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_internal.h
	imgui/imgui_widgets.cpp
	imgui/imgui_demo.cpp
	)

if(WIN32)
	set(SRC_IMGUI_IMPL
		windows/main.cpp
		windows/imgui_impl_dx12.cpp
		windows/imgui_impl_dx12.h
		windows/imgui_impl_win32.cpp
		windows/imgui_impl_win32.h
	)
	set(INCLUDE_IMGUI_IMPL
		${3RD_PATH}/gl3w
		${3RD_PATH}/glfw/include
		windows
	)
elseif(APPLE)
	set(SRC_IMGUI_IMPL
		mac/imgui_impl_metal.h
		mac/imgui_impl_metal.mm
		mac/imgui_impl_osx.h
		mac/imgui_impl_osx.mm
		mac/AppDelegate.h
		mac/AppDelegate.m
		mac/Renderer.h
		mac/Renderer.mm
		mac/ViewController.h
		mac/ViewController.mm
		mac/main.m
	)
	set(INCLUDE_IMGUI_IMPL mac)
endif()

set(SRC_PANEL
	panel/imgui_main.cpp
	panel/imgui_console.cpp
)

set(SRC_DEMO
	demo/demo_base.cpp
	demo/one_triangle.cpp
)

source_group(imgui FILES ${SRC_IMGUI})
source_group(imgui_impl FILES ${SRC_IMGUI_IMPL})
source_group(panel FILES ${SRC_PANEL})
source_group(demo FILES ${SRC_DEMO})

include_directories(
	imgui
	${INCLUDE_IMGUI_IMPL}
	panel
	demo
)

set(EXTRA_LIBS)
set(EXTRA_TARGET_OPTIONS)
set(EXTRA_TARGET_PROTERTIES)
set(EXTRA_TARGET_RESOURCES)

if(WIN32)
	set(EXTRA_LIBS d3d12 d3dcompiler dxgi)
	set(EXTRA_TARGET_OPTIONS WIN32)
	set(EXTRA_TARGET_PROTERTIES
		RUNTIME_OUTPUT_DIRECTORY "${BIN_DIR}"
		VS_DEBUGGER_WORKING_DIRECTORY "${BIN_DIR}"
	)
elseif(APPLE)
	find_library(METAL_FRAMEWORK Metal)
	find_library(MODELIO_FRAMEWORK ModelIO)
	find_library(METALKIT_FRAMEWORK MetalKit)
	set(EXTRA_LIBS
		${METAL_FRAMEWORK}
		${MODELIO_FRAMEWORK} 
		${METALKIT_FRAMEWORK}
	)
	mark_as_advanced(${EXTRA_LIBS})
	set(EXTRA_TARGET_RESOURCES
		${CMAKE_CURRENT_LIST_DIR}/mac/resource/Main.storyboard
	)
	set(EXTRA_TARGET_OPTIONS MACOSX_BUNDLE)
	set(EXTRA_TARGET_PROTERTIES
		MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_LIST_DIR}/mac/resource/Info-macOS.plist
		XCODE_ATTRIBUTE_CLANG_ENABLE_MODULES "YES"
		XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES"
		XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_WEAK "YES"
		XCODE_ATTRIBUTE_CLANG_MODULES_AUTOLINK "YES"
		XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "-"
		XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic"
		XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.wyc.pixpad"
		XCODE_ATTRIBUTE_PRODUCT_NAME "pixpad"
		RESOURCE ${EXTRA_TARGET_RESOURCES}
	)
endif()

add_executable(${PROJECT_NAME} ${EXTRA_TARGET_OPTIONS}
	${SRC_IMGUI}
	${SRC_IMGUI_IMPL}
	${SRC_PIXPAD}
	${SRC_PANEL}
	${SRC_DEMO}
)

set_target_properties(${PROJECT_NAME}
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${BIN_DIR}"
	${EXTRA_TARGET_PROTERTIES}
)

target_sources(${PROJECT_NAME} PUBLIC ${EXTRA_TARGET_RESOURCES})

target_link_libraries(${PROJECT_NAME} sparrow ${Boost_PROGRAM_OPTIONS_LIBRARY} ${EXTRA_LIBS})

