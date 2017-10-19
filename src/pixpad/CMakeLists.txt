cmake_minimum_required(VERSION 2.8)
project(pixpad)

set(SRC_IMGUI
	imgui/imgui.h
	imgui/imgui.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_internal.h
	imgui/imconfig.h
	imgui/imgui_impl_glfw_gl3.cpp
	imgui/imgui_impl_glfw_gl3.h
	imgui/imgui_demo.cpp
	)

set(SRC_GL3W
	${3RD_PATH}/gl3w/GL/gl3w.h
	${3RD_PATH}/gl3w/GL/gl3w.c
	${3RD_PATH}/gl3w/GL/glcorearb.h
)

set(SRC_APP
	app_config.h
	main.cpp
	console.cpp
	)

source_group(imgui FILES ${SRC_IMGUI})
source_group(gl3w FILES ${SRC_GL3W})

include_directories(imgui)
include_directories(${3RD_PATH}/gl3w)
include_directories(${3RD_PATH}/glfw/include)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	link_directories(${3RD_PATH}/glfw/lib-vc2015-64)
else()
	link_directories(${3RD_PATH}/glfw/lib-vc2015-32)
endif()

add_executable(${PROJECT_NAME}
	${SRC_IMGUI}
	${SRC_GL3W}
	${SRC_APP}
)

target_link_libraries(${PROJECT_NAME} glfw3)
target_link_libraries(${PROJECT_NAME} opengl32)
target_link_libraries(${PROJECT_NAME} gdi32)
target_link_libraries(${PROJECT_NAME} imm32)
