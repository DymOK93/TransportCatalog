cmake_minimum_required(VERSION 3.8)
project(Render)

set(CMAKE_CXX_STANDARD_REQUIRED 17)

set (
	RENDER_HEADER_FILES
		render.h
		render_settings_extractor.h
)
set (
	RENDER_SOURCE_FILES
		render.cpp
)

add_library(
	Render STATIC
		${RENDER_HEADER_FILES}
		${RENDER_SOURCE_FILES}
)
target_include_directories(
	Render PUBLIC 
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Render Svg)
target_link_libraries(Render Json)




