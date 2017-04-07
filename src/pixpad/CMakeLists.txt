cmake_minimum_required(VERSION 2.8)
project(pixpad_view)

find_library(LIB_GLEW glew32mx PATHS ${CMAKE_SOURCE_DIR}/3rd/lib)

set(src
	wmain.cpp
	stdafx.cpp
	win_app.cpp
	glrender.cpp
	logger.cpp
	util.cpp
	app_pixpad.cpp
)

set(hdr
	stdafx.h
	targetver.h
	win_app.h
	glrender.h
	logger.h
	log.h
	util.h
	app_pixpad.h
)

add_executable(${PROJECT_NAME} WIN32
	${src}
	${hdr}
)
target_link_libraries(${PROJECT_NAME} opengl32)
target_link_libraries(${PROJECT_NAME} glu32)
target_link_libraries(${PROJECT_NAME} ${LIB_GLEW})
target_link_libraries(${PROJECT_NAME} ${LIB_IMATH})
target_link_libraries(${PROJECT_NAME} ${LIB_PIXPAD})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/3rd/lib/glew32mx.dll" $<TARGET_FILE_DIR:${PROJECT_NAME}>
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/3rd/lib/Imath-2_2.dll" $<TARGET_FILE_DIR:${PROJECT_NAME}>
)