cmake_minimum_required (VERSION 3.8)
project ("Svg")

set(CMAKE_CXX_STANDARD_REQUIRED 17)

set(
	SVG_HEADER_FILES
		svg.h
		svg_primitives.h
)
set(
	SVG_SOURCE_FILES
		svg.cpp
		svg_primitives.cpp
)

add_library(
	Svg STATIC
		${SVG_HEADER_FILES}
		${SVG_SOURCE_FILES}
)
target_include_directories(
	Svg PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)