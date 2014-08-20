project(pixpad)
cmake_minimum_required(VERSION 2.8)

INCLUDE_DIRECTORIES(
	${pixpad_SOURCE_DIR}
	${pixpad_SOURCE_DIR}/mathex
)

# setup mathex
set(mathex_src
	mathex/mathex.cpp
	mathex/vecmath.cpp
)
set(mathex_hdr
	mathex/mathex.h
	mathex/vecmath.h
	mathex/arithmetic_helper.h
	mathex/fixedp.h
)
#add_library(libmathex STATIC ${mathex_src} ${mathex_hdr})

aux_source_directory(test TEST_LIST)
add_executable(${PROJECT_NAME} ${TEST_LIST} ${mathex_src} ${mathex_hdr})

