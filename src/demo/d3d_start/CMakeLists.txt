cmake_minimum_required(VERSION 3.10)
project(d3d_start)

set(GAME_SOURCE
	main.cpp
)

add_executable(${PROJECT_NAME} WIN32
	${GAME_SOURCE}
)

target_link_libraries(${PROJECT_NAME} engine)

set_target_properties(${PROJECT_NAME} 
	PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY ${RUNTIME_WORK_DIR}
	FOLDER demo
)
