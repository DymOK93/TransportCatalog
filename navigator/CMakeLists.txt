cmake_minimum_required(VERSION 3.8)
project(Navigator)

set(CMAKE_CXX_STANDARD_REQUIRED 17)

set (
	NAVIGATOR_HEADER_FILES
		graph.h
		navigator.h
		routing.h
		stats.h
)
set (
	NAVIGATOR_SOURCE_FILES
		stats.cpp
)

#Header-only
add_library(
	Navigator STATIC
		${NAVIGATOR_HEADER_FILES}
		${NAVIGATOR_SOURCE_FILES}
)
target_include_directories(
	Navigator PUBLIC 
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Navigator Geographic)




